#include "http_error.hpp"
#include "http_message.hpp"
#include <boost/beast.hpp>
#include "buffer.hpp"
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
template <bool isRequest, typename Body> struct parser {
  bingo::message<isRequest, Body, bingo::fields> msg;
  parser(bingo::message<isRequest, Body, bingo::fields> msg)
      : msg(std::move(msg)) {}
  void put_eof(std::error_code& ec){ec=std::error_code{};}
  void put(const char* buff,std::error_code& ec){
    
  }
};

template <bool isRequest, class Body>
void read_istream(std::istream &is, bingo::string_buffer &buffer,
                  bingo::message<isRequest, Body, bingo::fields> &msg, std::error_code& ec) {
  // Create the message parser
  //
  // Arguments passed to the parser's constructor are
  // forwarded to the message constructor. Here, we use
  // a move construction in case the caller has constructed
  // their message in a non-default way.
  //
  bingo::parser<isRequest, Body> p{std::move(msg)};

  do {
    // Extract whatever characters are presently available in the istream
    if (is.rdbuf()->in_avail() > 0) {
      // Get a mutable buffer sequence for writing
      auto const b =
          buffer.prepare(static_cast<std::size_t>(is.rdbuf()->in_avail()));

      // Now get everything we can from the istream
      buffer.commit(static_cast<std::size_t>(
          is.readsome(b, buffer.write_length())));
    } else if (buffer.capacity() == 0) {
      // Our buffer is empty and we need more characters,
      // see if we've reached the end of file on the istream
      if (!is.eof()) {
        // Get a mutable buffer sequence for writing
        auto const b = buffer.prepare(1024);

        // Try to get more from the istream. This might block.
        is.read(b, buffer.write_length());

        // If an error occurs on the istream then return it to the caller.
        if (is.fail() && !is.eof()) {
          // We'll just re-use io_error since std::istream has no error_code
          // interface.
          ec = (bingo::error::io_error);
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
    if (ec == bingo::error::need_more)
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

} // namespace bingo
