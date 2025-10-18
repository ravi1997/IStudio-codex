#include "front/ast_dump.h"

#include <sstream>
#include <string>
#include <string_view>

#include "front/ast.h"

namespace istudio::front {
namespace {

std::string escape_text(std::string_view value) {
  std::string result{};
  result.reserve(value.size());
  for (char ch : value) {
    if (ch == '"') {
      result += "\\\"";
    } else if (ch == '\\') {
      result += "\\\\";
    } else {
      result.push_back(ch);
    }
  }
  return result;
}

std::string escape_control_character(unsigned char ch) {
  constexpr char hex[] = "0123456789ABCDEF";
  std::string result = "\\u00";
  result.push_back(hex[(ch >> 4) & 0x0F]);
  result.push_back(hex[ch & 0x0F]);
  return result;
}

std::string json_escape(std::string_view value) {
  std::string result{};
  result.reserve(value.size());
  for (char raw : value) {
    switch (raw) {
      case '"':
        result += "\\\"";
        continue;
      case '\\':
        result += "\\\\";
        continue;
      case '\b':
        result += "\\b";
        continue;
      case '\f':
        result += "\\f";
        continue;
      case '\n':
        result += "\\n";
        continue;
      case '\r':
        result += "\\r";
        continue;
      case '\t':
        result += "\\t";
        continue;
      default:
        break;
    }

    const unsigned char ch = static_cast<unsigned char>(raw);
    if (ch < 0x20) {
      result += escape_control_character(ch);
      continue;
    }

    result.push_back(raw);
  }
  return result;
}

void dump_text_impl(const AstContext& context, NodeId id, const AstDumpOptions& options, std::string& out,
                    std::size_t depth) {
  const auto& node = context.node(id);
  std::string indent(depth * 2, ' ');
  out += indent;
  out += to_string(node.kind);

  if (options.include_ids) {
    out.push_back('#');
    out += std::to_string(node.id);
  }

  if (!node.value.empty()) {
    out += " value=\"";
    out += escape_text(node.value);
    out += '"';
  }

  if (options.include_spans) {
    std::ostringstream span_stream;
    span_stream << node.span;
    out += " span=";
    out += span_stream.str();
  }

  out.push_back('\n');

  for (NodeId child : node.children) {
    dump_text_impl(context, child, options, out, depth + 1);
  }
}

void dump_json_impl(const AstContext& context, NodeId id, const AstDumpOptions& options, std::string& out,
                    std::size_t indent) {
  const auto& node = context.node(id);
  const std::string indent_str(indent, ' ');
  out += indent_str;
  out += "{\n";

  const std::size_t inner_indent_value = indent + 2;
  const std::string inner_indent(inner_indent_value, ' ');

  std::size_t field_index = 0;
  auto add_field = [&](const std::string& field) {
    if (field_index > 0) {
      out += ",\n";
    }
    out += inner_indent;
    out += field;
    ++field_index;
  };

  if (options.include_ids) {
    add_field("\"id\": " + std::to_string(node.id));
  }

  add_field(std::string("\"kind\": \"") + std::string(to_string(node.kind)) + "\"");

  if (options.include_spans) {
    add_field("\"span\": {\"start\": " + std::to_string(node.span.start) + ", \"end\": " +
              std::to_string(node.span.end) + "}");
  }

  add_field(std::string("\"value\": \"") + json_escape(node.value) + "\"");

  if (field_index > 0) {
    out += ",\n";
  }

  out += inner_indent;
  out += "\"children\": [";

  if (!node.children.empty()) {
    out += "\n";
    for (std::size_t i = 0; i < node.children.size(); ++i) {
      dump_json_impl(context, node.children[i], options, out, inner_indent_value + 2);
      if (i + 1 < node.children.size()) {
        out += ",\n";
      } else {
        out += "\n";
      }
    }
    out += inner_indent;
    out += "]";
  } else {
    out += "]";
  }

  out += "\n";
  out += indent_str;
  out += "}";
}

}  // namespace

std::string dump_ast_text(const AstContext& context, NodeId root, const AstDumpOptions& options) {
  std::string output{};
  dump_text_impl(context, root, options, output, 0);
  return output;
}

std::string dump_ast_json(const AstContext& context, NodeId root, const AstDumpOptions& options) {
  std::string output{};
  dump_json_impl(context, root, options, output, 0);
  output.push_back('\n');
  return output;
}

}  // namespace istudio::front
