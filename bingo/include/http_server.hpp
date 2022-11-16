#pragma once
#include "bingo.hpp"
#include "http_parser.hpp"
#include "http_serializer.hpp"
#include "request_handler.hpp"
namespace bingo {
template <typename Derived>
struct http_server {
  std::string doc_root_;
  Derived& self() { return static_cast<Derived&>(*this); }
  auto validate_request() {
    return [](auto& buff) {
      std::stringstream stream;
      stream << buff.data();
      beast::flat_buffer buffer_;

      beast::http::request<beast::http::string_body> req_;
      beast::error_code ec{};
      read_istream(stream, buffer_, req_, ec);
      return unifex::just(req_);
    };
  }

  auto handle_request(auto doc_root) {
    return [=, doc_root = std::move(doc_root)](auto& req_) {
      std::strstream stream;

      auto resp = self().process_request(req_);
      beast::error_code ec{};
      write_ostream(stream, resp, ec);
      request_stop();  // close the coonection once request served
      return unifex::just(std::move(stream));
    };
  }
  auto make_error(http::status st, std::string_view error) {
    http::response<http::string_body> res{st, 11};
    res.set(http::field::server, "bingo:0.0.1");
    res.set(http::field::content_type, "text/plain");
    res.set(http::field::content_length, std::to_string(error.length()));
    res.body() = std::string{error.data(), error.length()};
    std::strstream stream;
    beast::error_code ec{};
    write_ostream(stream, res, ec);
    return std::move(stream);
  }
  auto error_to_response() {
    return [=](auto expn) {
      try {
        std::rethrow_exception(expn);
      } catch (const std::invalid_argument& e) {
        return unifex::just(make_error(
            http::status::forbidden,
            std::string(e.what()) + "Invalid Argument"));
      } catch (const std::exception& e) {
        return unifex::just(make_error(
            http::status::internal_server_error,
            std::string(e.what()) + " Server Error"));
      }
    };
  }
  auto let_stopped() {
    return []() {
      return unifex::just(std::string("Stopped"));
    };
  }
  auto send_response() {
    return [](auto& res) {
      return unifex::just(std::string(res.str()));
    };
  }
  void start(const std::string& doc_root, int port) {
    doc_root_ = doc_root;
    auto http_worker = [=](auto buff) {
      return unifex::just(buff) | unifex::let_value(validate_request()) |
          unifex::let_value(handle_request(doc_root)) |
          unifex::let_error(error_to_response()) |
          unifex::let_value(send_response());
    };
    make_listener("127.0.0.1", port) |
        listen_for_peer_to_peer_connection(
            context.get_scheduler(),
            stop_src.get_token(),
            context.get_scheduler(),
            std::move(http_worker)) |
        handle_error([&](std::exception& v) {
          stop_src.request_stop();
          printf("%s", v.what());
        }) |
        unifex::sync_wait();
  }
  std::string root_directory() const { return doc_root_; }
  void request_stop() { stop_src.request_stop(); }
  unifex::static_thread_pool context;
  unifex::inplace_stop_source stop_src;
};
}  // namespace bingo
