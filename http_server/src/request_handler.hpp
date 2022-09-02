#pragma once
#include "http_file_body.hpp"
namespace bingo {
inline std::string_view mime_type(std::string_view path) {
  using bingo::iequals;
  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == std::string_view::npos)
      return std::string_view{};
    return path.substr(pos);
  }();
  if (iequals(ext, ".htm"))
    return "text/html";
  if (iequals(ext, ".html"))
    return "text/html";
  if (iequals(ext, ".php"))
    return "text/html";
  if (iequals(ext, ".css"))
    return "text/css";
  if (iequals(ext, ".txt"))
    return "text/plain";
  if (iequals(ext, ".js"))
    return "application/javascript";
  if (iequals(ext, ".json"))
    return "application/json";
  if (iequals(ext, ".xml"))
    return "application/xml";
  if (iequals(ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals(ext, ".flv"))
    return "video/x-flv";
  if (iequals(ext, ".png"))
    return "image/png";
  if (iequals(ext, ".jpe"))
    return "image/jpeg";
  if (iequals(ext, ".jpeg"))
    return "image/jpeg";
  if (iequals(ext, ".jpg"))
    return "image/jpeg";
  if (iequals(ext, ".gif"))
    return "image/gif";
  if (iequals(ext, ".bmp"))
    return "image/bmp";
  if (iequals(ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff"))
    return "image/tiff";
  if (iequals(ext, ".tif"))
    return "image/tiff";
  if (iequals(ext, ".svg"))
    return "image/svg+xml";
  if (iequals(ext, ".svgz"))
    return "image/svg+xml";
  return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
inline std::string path_cat(std::string_view base, std::string_view path) {
  if (base.empty())
    return std::string(path);
  std::string result(base);
  char constexpr path_separator = '/';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());

  return result;
}

template <class Body, class Allocator>
auto handle_request(std::string_view doc_root,
                    http::request<Body, http::basic_fields<Allocator>> &&req) {
  // Returns a bad request response
  auto const bad_request = [&req](std::string_view why) {
    throw std::invalid_argument("Bad Request:" +std::string(why));
  };

  // Returns a not found response
  auto const not_found = [&req](std::string_view target) {
    throw file_not_found(target.data());
  };

  // Returns a server error response
  auto const server_error = [&req](std::string_view what) {
    throw std::exception();
  };

  // Make sure we can handle the method
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    bad_request("Unknown HTTP-method");

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != std::string_view::npos)
    bad_request("Illegal request-target");

  // Build the path to the requested file
  std::string path = path_cat(
      doc_root, std::string_view(req.target().data(), req.target().length()));
  if (req.target().back() == '/')
    path.append("index.html");

  // Attempt to open the file

  file_stdio file;
  file.open(path.c_str(), bingo::file_mode::scan);

  auto size = file.size();

//   Respond to HEAD request
  // if(req.method() == http::verb::head)
  // {
  //     bingo::response<std::string> res{bingo::status::ok, req.version()};
  //     res.set(bingo::field::server, "Bingo: 0.0.1");
  //     res.set(bingo::field::content_type, mime_type(path));
  //     res.set(bingo::field::content_length, std::to_string(size));
  //     res.set(bingo::field::keep_alive, req.keep_alive() ? "true" : "false");
  //     return res;
  // }

  // Respond to GET request
  file_body body{std::move(file)}; 
  bingo::response<file_body> res{std::move(body),bingo::status::ok,
                                   req.version()};
  res.set(bingo::field::server, "Bingo: 0.0.1");
  auto type = mime_type(path);
  res.set(bingo::field::content_type, type);
  res.set(bingo::field::content_length, std::to_string(size));
  res.set(bingo::field::keep_alive, req.keep_alive() ? "true" : "false");
 
  return res;
}
} // namespace bingo
