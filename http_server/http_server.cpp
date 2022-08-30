// Server side C/C++ program to demonstrate unifex based chat server
// programming
#include "bingo.hpp"
#include "http_parser.hpp"
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
auto handle_request() {
  return [](auto &&req_) {
    auto var = handle_request("/", std::move(req_));

    return unifex::just(std::string("Handled"));
  };
}
auto error_to_response() {
  return [](auto expn) {
    try {
      std::rethrow_exception(expn);
    } catch (const file_not_found& e) {
      return unifex::just(std::string(e.what()));
    }
    catch (const std::invalid_argument& e) {
      return unifex::just(std::string(e.what()));
    }
    catch (const std::exception& e) {
      return unifex::just(std::string("Internal Server Error"));
    }

  };
}
auto let_stopped() {
  return []() { return unifex::just(std::string("Stopped")); };
}
auto send_response() {
  return []() { return unifex::just(std::string("Sent")); };
}
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;

  unifex::static_thread_pool context;
  unifex::inplace_stop_source stop_src;
  int count = 0;
  auto worker = [](auto buff) {
    std::string newbuf;
    newbuf.resize(buff.read_length());
    std::copy(buff.data(), buff.data() + buff.read_length(), newbuf.data());
    if (buff.data() == std::string("throw\n"))
      throw application_error("application error");
    return unifex::just(std::move(newbuf));
  };

  auto http_worker = [](auto buff) {
    // auto resp = unifex::just(buff) |
    //             unifex::let_value(validate_request);
                // // process the request in a function that may be using a
                // // different execution context
                // | unifex::let_value(handle_request_l)
                // // If there are errors transform them into proper responses
                // | unifex::let_error(error_to_response)
                // // If the flow is cancelled, send back a proper response
                // // | unifex::let_stopped(stopped_to_response)
                // // write the result back to the client
                // | unifex::let_value(send_response);
    return unifex::just(buff)
           |unifex::let_value(validate_request())
           |unifex::let_value(handle_request())
           | unifex::let_error(error_to_response());
  
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
