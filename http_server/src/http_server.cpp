// Server side C/C++ program to demonstrate unifex based chat server
// programming
#include "bingo.hpp"
#include "http_parser.hpp"
#include "http_serializer.hpp"
#include "request_handler.hpp"
#include <filesystem>
#include <iostream>
#define PORT 8080

using namespace bingo;
auto validate_request() {
  return [](auto &buff) {
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
  return [doc_root=std::move(doc_root)](auto &&req_) {
    auto var = std::move(handle_request(doc_root, std::move(req_)));

    return unifex::just(std::move(var));
  };
}
auto make_error(bingo::status st, std::string_view error) {
  bingo::response<std::string> res{error.data(), st, 11};
  res.set(bingo::field::server, "bingo:0.0.1");
  res.set(bingo::field::content_type, "text/plain");
  res.set(bingo::field::content_length, std::to_string(error.length()));
  return res;
}
auto error_to_response() {
  return [](auto expn) {
    try {
      std::rethrow_exception(expn);
    } catch (const file_not_found &e) {
      return unifex::just(make_error(bingo::status::not_found,
                                     std::string(e.what()) + "Not Found"));
    } catch (const std::invalid_argument &e) {
      return unifex::just(
          make_error(bingo::status::forbidden,
                     std::string(e.what()) + "Invalid Argument"));
    } catch (const std::exception &e) {
      return unifex::just(make_error(bingo::status::internal_server_error,
                                     std::string(e.what()) + "Server Error"));
    }
  };
}
auto let_stopped() {
  return []() { return unifex::just(std::string("Stopped")); };
}
auto send_response() {
  return [](auto &res) { return unifex::just(serialize(res)); };
}
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  namespace fs = std::filesystem;
  std::string doc_root=fs::current_path().c_str();
  if(argc >1){
    doc_root=argv[1];
  }
  unifex::static_thread_pool context;
  unifex::inplace_stop_source stop_src;

  auto http_worker = [=](auto buff) {
    return unifex::just(buff) | unifex::let_value(validate_request()) |
           unifex::let_value(handle_request(doc_root)) |
           unifex::let_error(error_to_response()) |
           unifex::let_value(send_response());
  };
  make_listener("127.0.0.1", PORT) |
      listen_for_peer_to_peer_connection(
          context.get_scheduler(), stop_src.get_token(),
          context.get_scheduler(), std::move(http_worker)) |
      handle_error([&](std::exception &v) {
        stop_src.request_stop();
        printf("%s", v.what());
      }) |
      unifex::sync_wait();

  return 0;
}
