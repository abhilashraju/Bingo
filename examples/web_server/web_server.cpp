// Server side C/C++ program to demonstrate unifex based chat server
// programming

#include <http_error.hpp>
#include <web_server.hpp>

#include <filesystem>
#include <iostream>
#define PORT 8089

using namespace bingo;

int main(int argc, char const* argv[]) {
  (void)argc;
  (void)argv;
  namespace fs = std::filesystem;
  std::string doc_root = fs::current_path().c_str();
  if (argc > 1) {
    doc_root = argv[1];
  }
  web_server server;
  auto plain_text_handler = [](auto func) {
    return [func = std::move(func)](auto& req, auto& httpfunc) {
      http::response<http::string_body> resp{http::status::ok, req.version()};
      resp.set(http::field::content_type, "text/plain");
      resp.body() = func(req, httpfunc);
      resp.set(
          http::field::content_length, std::to_string(resp.body().length()));
      return resp;
    };
  };
  server.add_handler(
      {"/hello", http::verb::get}, [](auto& req, auto& httpfunc) {
        http::response<http::string_body> resp{http::status::ok, req.version()};
        resp.set(http::field::content_type, "text/plain");
        resp.body() = "hello world";
        resp.set(
            http::field::content_length, std::to_string(resp.body().length()));
        return resp;
      });

  server.add_handler(
      {"/greetings", http::verb::get},
      plain_text_handler([](auto& req, auto& httpfunc) {
        return "Hello " + httpfunc["name"] + "!!!";
      }));
  server.start(doc_root, PORT);

  return 0;
}
