#include "backends/cpp/cpp_backend.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace istudio::backends::cpp {
namespace {

std::string sanitize_for_filename(std::string_view name) {
  std::string result;
  result.reserve(name.size());
  for (const char ch : name) {
    const auto uch = static_cast<unsigned char>(ch);
    if (std::isalnum(uch) != 0) {
      result.push_back(static_cast<char>(std::tolower(uch)));
    } else if (!result.empty() && result.back() != '_') {
      result.push_back('_');
    }
  }
  if (result.empty()) {
    return "module";
  }
  return result;
}

class NamespaceEmitter {
 public:
  explicit NamespaceEmitter(std::string ns) : ns_(std::move(ns)) {}

  void open(std::ostringstream& out) const {
    if (ns_.empty()) {
      return;
    }
    out << "namespace " << ns_ << " {\n\n";
  }

  void close(std::ostringstream& out) const {
    if (ns_.empty()) {
      return;
    }
    out << "}  // namespace " << ns_ << "\n";
  }

  [[nodiscard]] const std::string& value() const noexcept { return ns_; }

 private:
  std::string ns_;
};

class CppEmitter {
 public:
  CppEmitter(const ir::IRModule& module, CppBackendOptions options)
      : module_(module),
        options_(std::move(options)),
        ns_emitter_(options_.namespace_name),
        sanitized_name_(sanitize_for_filename(module_.name())) {
    if (sanitized_name_.empty()) {
      sanitized_name_ = "module";
    }
    header_filename_ = sanitized_name_ + options_.header_suffix;
    source_filename_ = sanitized_name_ + options_.source_suffix;
  }

  [[nodiscard]] std::vector<GeneratedFile> emit() {
    collect_includes();

    std::vector<GeneratedFile> files;
    if (options_.emit_header) {
      files.push_back({header_filename_, build_header()});
    }
    if (options_.emit_source) {
      files.push_back({source_filename_, build_source()});
    }
    return files;
  }

 private:
  void collect_includes_for_type(const ir::IRType& type) {
    using istudio::ir::IRTypeKind;
    switch (type.kind) {
      case IRTypeKind::I32:
      case IRTypeKind::I64:
        header_includes_.insert("<cstdint>");
        break;
      case IRTypeKind::String:
        header_includes_.insert("<string>");
        break;
      case IRTypeKind::Struct:
      case IRTypeKind::Generic:
        break;
      case IRTypeKind::F32:
      case IRTypeKind::F64:
      case IRTypeKind::Bool:
      case IRTypeKind::Void:
        break;
    }
    for (const auto& arg : type.type_arguments) {
      collect_includes_for_type(arg);
    }
  }

  void collect_includes() {
    for (const auto& record : module_.structs()) {
      for (const auto& field : record.fields) {
        collect_includes_for_type(field.type);
      }
    }
    for (const auto& fn : module_.functions()) {
      collect_includes_for_type(fn.return_type);
      for (const auto& param : fn.parameters) {
        collect_includes_for_type(param.type);
      }
    }
  }

  std::string type_to_string(const ir::IRType& type) {
    collect_includes_for_type(type);
    using istudio::ir::IRTypeKind;
    switch (type.kind) {
      case IRTypeKind::Void:
        return "void";
      case IRTypeKind::I32:
        return "std::int32_t";
      case IRTypeKind::I64:
        return "std::int64_t";
      case IRTypeKind::F32:
        return "float";
      case IRTypeKind::F64:
        return "double";
      case IRTypeKind::Bool:
        return "bool";
      case IRTypeKind::String:
        return "std::string";
      case IRTypeKind::Generic:
        return type.name;
      case IRTypeKind::Struct: {
        std::ostringstream oss;
        oss << type.name;
        if (!type.type_arguments.empty()) {
          oss << '<';
          for (std::size_t i = 0; i < type.type_arguments.size(); ++i) {
            if (i != 0) {
              oss << ", ";
            }
            oss << type_to_string(type.type_arguments[i]);
          }
          oss << '>';
        }
        return oss.str();
      }
    }
    return "void";
  }

  std::string format_template_parameters(const std::vector<std::string>& params) const {
    if (params.empty()) {
      return {};
    }
    std::ostringstream oss;
    oss << "template <";
    for (std::size_t i = 0; i < params.size(); ++i) {
      if (i != 0) {
        oss << ", ";
      }
      oss << "typename " << params[i];
    }
    oss << ">\n";
    return oss.str();
  }

  std::string format_parameter_list(const std::vector<ir::IRParameter>& params) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < params.size(); ++i) {
      if (i != 0) {
        oss << ", ";
      }
      oss << type_to_string(params[i].type) << ' ' << params[i].name;
    }
    return oss.str();
  }

  void emit_struct(const ir::IRStruct& record, std::ostringstream& out) {
    if (!record.template_params.empty()) {
      out << format_template_parameters(record.template_params);
    }
    if (!record.is_public) {
      out << "// internal\n";
    }
    out << "struct " << record.name << " {\n";
    for (const auto& field : record.fields) {
      out << "  " << type_to_string(field.type) << ' ' << field.name << ";\n";
    }
    out << "};\n\n";
  }

  void emit_function_declaration(const ir::IRFunction& fn, std::ostringstream& out) {
    if (!fn.template_params.empty()) {
      out << format_template_parameters(fn.template_params);
    }
    out << type_to_string(fn.return_type) << ' ' << fn.name << '(' << format_parameter_list(fn.parameters)
        << ");\n\n";
  }

  std::string emit_binary_op(const ir::IRValue& inst, std::string_view symbol) {
    if (inst.operands.size() != 2) {
      return "// unsupported operand count for '" + inst.op + "'";
    }
    std::ostringstream line;
    if (!inst.result.empty()) {
      line << "auto " << inst.result << " = ";
    }
    line << inst.operands[0] << ' ' << symbol << ' ' << inst.operands[1] << ';';
    return line.str();
  }

  std::vector<std::string> translate_instructions(const ir::IRFunction& fn) {
    std::vector<std::string> lines;
    lines.reserve(fn.instructions.size());

    for (const auto& inst : fn.instructions) {
      if (inst.is_constant) {
        if (inst.result.empty()) {
          lines.emplace_back("// constant value discarded (no target)");
        } else {
          lines.emplace_back("auto " + inst.result + " = " + std::to_string(inst.constant_value) + ";");
        }
        continue;
      }

      if (inst.op == "ret" || inst.op == "return") {
        if (inst.operands.empty()) {
          lines.emplace_back("return;");
        } else {
          lines.emplace_back("return " + inst.operands.front() + ";");
        }
        continue;
      }

      if (inst.op == "const") {
        if (inst.operands.empty()) {
          lines.emplace_back("// const missing operand");
        } else if (inst.result.empty()) {
          lines.emplace_back(inst.operands.front() + ";");
        } else {
          lines.emplace_back("auto " + inst.result + " = " + inst.operands.front() + ";");
        }
        continue;
      }

      if (inst.op == "add") {
        lines.emplace_back(emit_binary_op(inst, "+"));
        continue;
      }
      if (inst.op == "sub") {
        lines.emplace_back(emit_binary_op(inst, "-"));
        continue;
      }
      if (inst.op == "mul") {
        lines.emplace_back(emit_binary_op(inst, "*"));
        continue;
      }
      if (inst.op == "div") {
        lines.emplace_back(emit_binary_op(inst, "/"));
        continue;
      }
      if (inst.op == "mod") {
        lines.emplace_back(emit_binary_op(inst, "%"));
        continue;
      }
      if (inst.op == "neg") {
        if (inst.operands.size() != 1) {
          lines.emplace_back("// neg expects one operand");
        } else if (inst.result.empty()) {
          lines.emplace_back("-" + inst.operands.front() + ';');
        } else {
          lines.emplace_back("auto " + inst.result + " = -" + inst.operands.front() + ";");
        }
        continue;
      }

      if (inst.op == "call") {
        if (inst.operands.empty()) {
          lines.emplace_back("// call missing callee");
          continue;
        }
        std::ostringstream line;
        if (!inst.result.empty()) {
          line << "auto " << inst.result << " = ";
        }
        line << inst.operands.front() << '(';
        for (std::size_t i = 1; i < inst.operands.size(); ++i) {
          if (i != 1) {
            line << ", ";
          }
          line << inst.operands[i];
        }
        line << ");";
        lines.emplace_back(line.str());
        continue;
      }

      lines.emplace_back("// unsupported op '" + inst.op + "'");
    }

    if (lines.empty()) {
      lines.emplace_back("// TODO: provide implementation");
    }

    return lines;
  }

  void emit_function_definition(const ir::IRFunction& fn, std::ostringstream& out) {
    if (!fn.template_params.empty()) {
      out << format_template_parameters(fn.template_params);
    }
    out << type_to_string(fn.return_type) << ' ' << fn.name << '(' << format_parameter_list(fn.parameters)
        << ") {\n";
    const auto body_lines = translate_instructions(fn);
    for (const auto& line : body_lines) {
      out << "  " << line << '\n';
    }
    out << "}\n\n";
  }

  std::string build_header() {
    std::ostringstream out;

    out << "#pragma once\n\n";
    if (!header_includes_.empty()) {
      for (const auto& include : header_includes_) {
        out << "#include " << include << '\n';
      }
      out << '\n';
    }

    ns_emitter_.open(out);
    for (const auto& record : module_.structs()) {
      emit_struct(record, out);
    }
    for (const auto& fn : module_.functions()) {
      emit_function_declaration(fn, out);
    }
    ns_emitter_.close(out);
    return out.str();
  }

  std::string build_source() {
    std::ostringstream out;
    if (options_.emit_header) {
      out << "#include \"" << header_filename_ << "\"\n\n";
    } else {
      for (const auto& include : header_includes_) {
        out << "#include " << include << '\n';
      }
      out << '\n';
    }

    ns_emitter_.open(out);
    for (const auto& fn : module_.functions()) {
      emit_function_definition(fn, out);
    }
    ns_emitter_.close(out);
    return out.str();
  }

  const ir::IRModule& module_;
  CppBackendOptions options_;
  NamespaceEmitter ns_emitter_;
  std::set<std::string> header_includes_{};
  std::string sanitized_name_;
  std::string header_filename_;
  std::string source_filename_;
};

}  // namespace

CppBackend::CppBackend(CppBackendOptions options) : options_(std::move(options)) {}

std::string CppBackend::name() const { return "cpp"; }

std::vector<GeneratedFile> CppBackend::emit(const ir::IRModule& module, const TargetProfile& profile) {
  (void)profile;
  CppEmitter emitter(module, options_);
  return emitter.emit();
}

}  // namespace istudio::backends::cpp
