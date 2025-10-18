#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace istudio::lsp {

// MessageReader extracts Language Server Protocol JSON-RPC payloads from an input stream.
class MessageReader {
 public:
  // Returns true when a complete payload is read into |out|, false on EOF or malformed headers.
  bool read_message(std::istream& in, std::string& out);
};

// MessageWriter serializes JSON-RPC payloads back to the client using the LSP framing protocol.
class MessageWriter {
 public:
  void write_message(std::ostream& out, std::string_view payload) const;
};

}  // namespace istudio::lsp
