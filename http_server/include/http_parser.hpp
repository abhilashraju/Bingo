#include "http_error.hpp"
#include "http_message.hpp"
#include <boost/beast.hpp>
#include <istream>
namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;
namespace bingo {
template <class Allocator, bool isRequest, class Body>
void read_istream(
    std::istream &is, beast::basic_flat_buffer<Allocator> &buffer,
    beast::http::message<isRequest, Body, beast::http::fields> &msg,
    beast::error_code &ec) {
  // Create the message parser
  //
  // Arguments passed to the parser's constructor are
  // forwarded to the message constructor. Here, we use
  // a move construction in case the caller has constructed
  // their message in a non-default way.
  //
  beast::http::parser<isRequest, Body> p{std::move(msg)};

  do {
    // Extract whatever characters are presently available in the istream
    if (is.rdbuf()->in_avail() > 0) {
      // Get a mutable buffer sequence for writing
      auto const b =
          buffer.prepare(static_cast<std::size_t>(is.rdbuf()->in_avail()));

      // Now get everything we can from the istream
      buffer.commit(static_cast<std::size_t>(
          is.readsome(reinterpret_cast<char *>(b.data()), b.size())));
    } else if (buffer.size() == 0) {
      // Our buffer is empty and we need more characters,
      // see if we've reached the end of file on the istream
      if (!is.eof()) {
        // Get a mutable buffer sequence for writing
        auto const b = buffer.prepare(1024);

        // Try to get more from the istream. This might block.
        is.read(reinterpret_cast<char *>(b.data()), b.size());

        // If an error occurs on the istream then return it to the caller.
        if (is.fail() && !is.eof()) {
          // We'll just re-use io_error since std::istream has no error_code
          // interface.
          ec = beast::error::timeout; //(std::errc::io_error);
          return;
        }

        // Commit the characters we got to the buffer.
        buffer.commit(static_cast<std::size_t>(is.gcount()));
      } else {
        // Inform the parser that we've reached the end of the istream.
        p.put_eof(ec);
        if (ec)
          return;
        break;
      }
    }

    // Write the data to the parser
    auto const bytes_used = p.put(buffer.data(), ec);

    // This error means that the parser needs additional octets.
    if (ec == beast::http::error::need_more)
      ec = {};
    if (ec)
      return;

    // Consume the buffer octets that were actually parsed.
    buffer.consume(bytes_used);
  } while (!p.is_done());

  // Transfer ownership of the message container in the parser to the caller.
  msg = p.release();
}
inline bool iequals(std::string_view lhs, std::string_view rhs) {
  auto n = lhs.size();
  if (rhs.size() != n)
    return false;
  auto p1 = lhs.data();
  auto p2 = rhs.data();
  char a, b;
  // fast loop
  while (n--) {
    a = *p1++;
    b = *p2++;
    if (a != b)
      goto slow;
  }
  return true;
slow:
  do {
    if (std::tolower(a) != std::tolower(b))
      return false;
    a = *p1++;
    b = *p2++;
  } while (n--);
  return true;
}

// inline bool
// iless::operator()(
//     string_view lhs,
//     string_view rhs) const
// {
//     using std::begin;
//     using std::end;
//     return std::lexicographical_compare(
//         begin(lhs), end(lhs), begin(rhs), end(rhs),
//         [](char c1, char c2)
//         {
//             return detail::ascii_tolower(c1) < detail::ascii_tolower(c2);
//         }
//     );
// }

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
#ifdef BOOST_MSVC
  char constexpr path_separator = '\\';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  for (auto &c : result)
    if (c == '/')
      c = path_separator;
#else
  char constexpr path_separator = '/';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

template <class Body, class Allocator>
auto handle_request(std::string_view doc_root,
                    http::request<Body, http::basic_fields<Allocator>> &&req) {
  // Returns a bad request response
  auto const bad_request = [&req](std::string_view why) {
    // http::response<http::string_body> res{http::status::bad_request,
    // req.version()}; res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    // res.set(http::field::content_type, "text/html");
    // res.keep_alive(req.keep_alive());
    // res.body() = std::string(why);
    // res.prepare_payload();
    // return res;
    throw std::invalid_argument("bad request");
  };

  // Returns a not found response
  auto const not_found = [&req](std::string_view target) {
    // http::response<http::string_body> res{http::status::not_found,
    // req.version()}; res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    // res.set(http::field::content_type, "text/html");
    // res.keep_alive(req.keep_alive());
    // res.body() = "The resource '" + std::string(target) + "' was not found.";
    // res.prepare_payload();
    // return res;
    throw file_not_found(target.data());
  };

  // Returns a server error response
  auto const server_error = [&req](std::string_view what) {
    // http::response<http::string_body>
    // res{http::status::internal_server_error, req.version()};
    // res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    // res.set(http::field::content_type, "text/html");
    // res.keep_alive(req.keep_alive());
    // res.body() = "An error occurred: '" + std::string(what) + "'";
    // res.prepare_payload();
    // return res;
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
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if (ec == beast::errc::no_such_file_or_directory)

    not_found(std::string_view(req.target().data(), req.target().length()));

  // Handle an unknown error
  if (ec)
    server_error(ec.message());

  // Cache the size since we need it after the move
  auto const size = body.size();

  // Respond to HEAD request
  // if(req.method() == http::verb::head)
  // {
  //     http::response<http::empty_body> res{http::status::ok, req.version()};
  //     res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  //     res.set(http::field::content_type, mime_type(path));
  //     res.content_length(size);
  //     res.keep_alive(req.keep_alive());
  //     return res;
  // }

  // Respond to GET request

  bingo::response<std::string> res{"response", bingo::status::ok,
                                   req.version()};
  res.set(bingo::field::server, "bingo:0.0.1");
  auto type = mime_type(path);
  res.set(bingo::field::content_type, type);
  res. set(bingo::field::content_length, std::to_string(size));
  res.set(bingo::field::keep_alive,req.keep_alive()?"true":"false");
  return res;
}
} // namespace bingo
