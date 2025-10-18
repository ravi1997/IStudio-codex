#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lsp/message_io.h"
#include "lsp/server.h"

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

std::string wrap_message(const std::string& body) {
  std::ostringstream oss;
  oss << "Content-Length: " << body.size() << "\r\n\r\n" << body;
  return oss.str();
}

std::string strip_whitespace(std::string text) {
  text.erase(
      std::remove_if(text.begin(), text.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }),
      text.end());
  return text;
}

void test_reader_extracts_payload() {
  const std::string payload = R"({"jsonrpc":"2.0","id":1,"method":"ping"})";
  std::istringstream input(wrap_message(payload));
  istudio::lsp::MessageReader reader;
  std::string decoded;

  expect(reader.read_message(input, decoded), "LSP reader should extract payload");
  expect(decoded == payload, "Extracted payload should match input");
}

void test_server_handles_initialize_shutdown() {
  const std::string initialize_request =
      R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"processId":1,"rootUri":null}})";
  const std::string shutdown_request = R"({"jsonrpc":"2.0","id":2,"method":"shutdown","params":null})";
  const std::string exit_notification = R"({"jsonrpc":"2.0","method":"exit"})";

  std::istringstream input(
      wrap_message(initialize_request) + wrap_message(shutdown_request) + wrap_message(exit_notification));
  std::ostringstream output;

  istudio::lsp::Server server{};
  const int exit_code = server.run(input, output);
  expect(exit_code == 0, "Server should exit with code 0 after graceful shutdown");

  std::istringstream response_stream(output.str());
  istudio::lsp::MessageReader reader;
  std::string payload;

  expect(reader.read_message(response_stream, payload), "Initialize response should be emitted");
  const std::string compact_init = strip_whitespace(payload);
  expect(compact_init.find("\"id\":1") != std::string::npos, "Initialize response should target request id 1");
  expect(compact_init.find("\"capabilities\"") != std::string::npos, "Initialize response should expose capabilities");

  expect(reader.read_message(response_stream, payload), "Shutdown response should be emitted");
  const std::string compact_shutdown = strip_whitespace(payload);
  expect(compact_shutdown.find("\"id\":2") != std::string::npos, "Shutdown response should target request id 2");
  expect(compact_shutdown.find("\"result\":null") != std::string::npos,
         "Shutdown response should include null result");

  expect(!reader.read_message(response_stream, payload), "No further responses should be emitted");
}

}  // namespace

void run_lsp_tests() {
  test_reader_extracts_payload();
  test_server_handles_initialize_shutdown();
  std::cout << "All LSP tests passed\n";
}
