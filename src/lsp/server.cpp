#include "lsp/server.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include "lsp/message_io.h"
#include "support/version.h"

namespace {

bool is_space(char ch) {
  return std::isspace(static_cast<unsigned char>(ch)) != 0;
}

bool parse_string_literal(const std::string& payload, std::size_t& index, std::string& out) {
  if (index >= payload.size() || payload[index] != '"') {
    return false;
  }

  ++index;  // consume opening quote
  out.clear();

  while (index < payload.size()) {
    char ch = payload[index];
    if (ch == '"') {
      ++index;  // consume closing quote
      return true;
    }

    if (ch == '\\') {
      ++index;
      if (index >= payload.size()) {
        return false;
      }
      char escaped = payload[index];
      switch (escaped) {
        case '"':
        case '\\':
        case '/':
          out.push_back(escaped);
          break;
        case 'b':
          out.push_back('\b');
          break;
        case 'f':
          out.push_back('\f');
          break;
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;
        default:
          return false;
      }
      ++index;
      continue;
    }

    out.push_back(ch);
    ++index;
  }

  return false;
}

struct PropertyResult {
  bool found = false;
  bool is_string = false;
  std::string value;
};

PropertyResult extract_property(const std::string& payload, std::string_view key) {
  PropertyResult result;

  const std::string needle = "\"" + std::string(key) + "\"";
  std::size_t key_pos = payload.find(needle);
  if (key_pos == std::string::npos) {
    return result;
  }

  std::size_t colon_pos = payload.find(':', key_pos + needle.size());
  if (colon_pos == std::string::npos) {
    return result;
  }

  std::size_t value_start = payload.find_first_not_of(" \t\r\n", colon_pos + 1);
  if (value_start == std::string::npos) {
    return result;
  }

  if (payload[value_start] == '"') {
    result.is_string = true;
    std::size_t cursor = value_start;
    if (!parse_string_literal(payload, cursor, result.value)) {
      result.found = false;
      return result;
    }
    result.found = true;
    return result;
  }

  std::size_t value_end = value_start;
  while (value_end < payload.size()) {
    char ch = payload[value_end];
    if (ch == ',' || ch == '}' || ch == '\r' || ch == '\n') {
      break;
    }
    ++value_end;
  }

  result.value = payload.substr(value_start, value_end - value_start);
  while (!result.value.empty() && is_space(result.value.back())) {
    result.value.pop_back();
  }
  result.found = true;
  result.is_string = false;
  return result;
}

std::string json_escape(std::string_view text) {
  std::string result;
  result.reserve(text.size() + 8);

  constexpr char hex_digits[] = "0123456789ABCDEF";

  for (char ch : text) {
    const unsigned char raw_ch = static_cast<unsigned char>(ch);
    switch (ch) {
      case '"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\b':
        result += "\\b";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        if (raw_ch < 0x20) {
          result += "\\u00";
          result.push_back(hex_digits[(raw_ch >> 4) & 0x0F]);
          result.push_back(hex_digits[raw_ch & 0x0F]);
        } else {
          result.push_back(ch);
        }
    }
  }

  return result;
}

}  // namespace

namespace istudio::lsp {

Server::Server(ServerOptions options) : options_(options) {}

int Server::run(std::istream& in, std::ostream& out) {
  MessageReader reader;
  std::string payload;

  while (reader.read_message(in, payload)) {
    ParsedMessage message;
    if (!parse_message(payload, message)) {
      send_parse_error(out);
      continue;
    }

    handle_message(message, out);
    if (exit_requested_) {
      return exit_code_;
    }
  }

  if (exit_requested_) {
    return exit_code_;
  }

  return shutdown_received_ ? 0 : 0;
}

bool Server::parse_message(const std::string& payload, ParsedMessage& message) const {
  message = ParsedMessage{};

  const PropertyResult jsonrpc = extract_property(payload, "jsonrpc");
  if (!jsonrpc.found || !jsonrpc.is_string || jsonrpc.value != "2.0") {
    return false;
  }

  const PropertyResult method = extract_property(payload, "method");
  if (!method.found || !method.is_string) {
    return false;
  }

  message.method = method.value;

  const PropertyResult id = extract_property(payload, "id");
  if (id.found) {
    message.has_id = true;
    message.id.value = id.value;
    message.id.is_string = id.is_string;
  }

  message.valid = true;
  return true;
}

void Server::handle_message(const ParsedMessage& message, std::ostream& out) {
  if (!message.valid) {
    send_error(std::nullopt, -32600, "Invalid Request", out);
    return;
  }

  if (message.has_id) {
    handle_request(message, out);
    return;
  }

  handle_notification(message.method);
}

void Server::handle_request(const ParsedMessage& message, std::ostream& out) {
  if (!message.has_id) {
    send_error(std::nullopt, -32600, "Invalid Request", out);
    return;
  }

  if (message.method == "initialize") {
    const std::string result = make_initialize_result();
    send_response(message.id, result, out);
    return;
  }

  if (message.method == "shutdown") {
    shutdown_received_ = true;
    send_response(message.id, "null", out);
    return;
  }

  send_error(message.id, -32601, "Method not implemented", out);
}

void Server::handle_notification(std::string_view method) {
  if (method == "exit") {
    exit_requested_ = true;
    exit_code_ = shutdown_received_ ? 0 : 1;
  }
  // Other notifications (initialized, didOpen, etc.) are intentionally ignored in the scaffold.
}

void Server::send_response(const ResponseId& id, std::string_view result_json, std::ostream& out) {
  MessageWriter writer;
  std::ostringstream oss;
  oss << "{\"jsonrpc\":\"2.0\",\"id\":";
  if (id.is_string) {
    oss << "\"" << json_escape(id.value) << "\"";
  } else {
    oss << id.value;
  }
  oss << ",\"result\":" << result_json << "}";
  writer.write_message(out, oss.str());
}

void Server::send_error(std::optional<ResponseId> id, int code, std::string_view message, std::ostream& out) {
  MessageWriter writer;
  std::ostringstream oss;
  oss << "{\"jsonrpc\":\"2.0\",\"id\":";
  if (id.has_value()) {
    if (id->is_string) {
      oss << "\"" << json_escape(id->value) << "\"";
    } else {
      oss << id->value;
    }
  } else {
    oss << "null";
  }
  oss << ",\"error\":{\"code\":" << code << ",\"message\":\"" << json_escape(message) << "\"}}";
  writer.write_message(out, oss.str());
}

void Server::send_parse_error(std::ostream& out) {
  send_error(std::nullopt, -32700, "Parse error", out);
}

std::string Server::make_initialize_result() const {
  std::ostringstream oss;
  oss << "{\"capabilities\":{"
      << "\"textDocumentSync\":{\"openClose\":true,\"change\":1,\"save\":{\"includeText\":false}},"
      << "\"hoverProvider\":false,"
      << "\"definitionProvider\":false,"
      << "\"referencesProvider\":false,"
      << "\"documentSymbolProvider\":false,"
      << "\"completionProvider\":{}},"
      << "\"serverInfo\":{\"name\":\""
      << json_escape("IStudio Language Server")
      << "\",\"version\":\""
      << json_escape(istudio::support::current_version())
      << "\"}}";
  return oss.str();
}

}  // namespace istudio::lsp
