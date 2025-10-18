#pragma once

#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace istudio::lsp {

struct ServerOptions {
  bool exit_on_shutdown = true;
};

class Server {
 public:
 explicit Server(ServerOptions options = {});

  // Runs the server loop using the provided streams. Returns the exit code dictated by the LSP spec.
  int run(std::istream& in, std::ostream& out);

 private:
  struct ResponseId {
    std::string value;
    bool is_string = false;
  };

  struct ParsedMessage {
    bool valid = false;
    bool has_id = false;
    ResponseId id;
    std::string method;
  };

  bool parse_message(const std::string& payload, ParsedMessage& message) const;
  void handle_message(const ParsedMessage& message, std::ostream& out);
  void handle_request(const ParsedMessage& message, std::ostream& out);
  void handle_notification(std::string_view method);
  void send_response(const ResponseId& id, std::string_view result_json, std::ostream& out);
  void send_error(std::optional<ResponseId> id, int code, std::string_view message, std::ostream& out);
  void send_parse_error(std::ostream& out);
  std::string make_initialize_result() const;

  bool shutdown_received_ = false;
  bool exit_requested_ = false;
  int exit_code_ = 0;
  ServerOptions options_;
};

}  // namespace istudio::lsp
