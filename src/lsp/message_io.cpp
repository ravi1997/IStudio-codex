#include "lsp/message_io.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {

bool is_space(unsigned char ch) {
  return std::isspace(ch) != 0;
}

void trim_in_place(std::string& text) {
  auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) { return is_space(ch); });
  if (first == text.end()) {
    text.clear();
    return;
  }
  auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) { return is_space(ch); }).base();
  text.assign(first, last);
}

void lowercase_in_place(std::string& text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
}

}  // namespace

namespace istudio::lsp {

bool MessageReader::read_message(std::istream& in, std::string& out) {
  std::string line;
  std::size_t content_length = 0;
  bool saw_any_header = false;

  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) {
      break;
    }

    saw_any_header = true;

    auto colon = line.find(':');
    if (colon == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    trim_in_place(key);
    trim_in_place(value);
    lowercase_in_place(key);

    if (key == "content-length") {
      try {
        content_length = static_cast<std::size_t>(std::stoul(value));
      } catch (const std::exception&) {
        return false;
      }
    }
  }

  if (!saw_any_header && !in.good()) {
    return false;
  }

  if (content_length == 0) {
    return false;
  }

  out.resize(content_length);
  in.read(out.data(), static_cast<std::streamsize>(content_length));
  if (in.gcount() != static_cast<std::streamsize>(content_length)) {
    out.clear();
    return false;
  }

  if (in.peek() == '\r') {
    in.get();
    if (in.peek() == '\n') {
      in.get();
    }
  }

  return true;
}

void MessageWriter::write_message(std::ostream& out, std::string_view payload) const {
  out << "Content-Length: " << payload.size() << "\r\n\r\n";
  out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
  out.flush();
}

}  // namespace istudio::lsp
