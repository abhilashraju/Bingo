#pragma once
#include "http_server.hpp"
#include "http_target_parser.hpp"
#include "http_error.hpp"
#include <map>
namespace bingo {
struct web_server : public http_server<web_server> {

  struct handler_base {
    using Request=http::request<http::string_body>;
    virtual http::response<http::string_body> handle(Request& req,const http_function &vw) = 0;
    virtual ~handler_base() {}
  };

  template <typename HandlerFunc> struct handler : handler_base {
    HandlerFunc func;
    handler(HandlerFunc fun) : func(std::move(fun)) {}

    http::response<http::string_body> handle(Request& req,const http_function &params) override {
      return func(req,params);
    }
  };
  
  template <typename FUNC> void add_handler(http::verb v,std::string_view path, FUNC&& h) {
    auto& handlers=handler_for_verb(v);
    handlers[std::string{path.data(), path.length()}] =
        std::make_unique<handler<FUNC>>(std::move(h));
  }
  template <typename FUNC> void add_get_handler(std::string_view path, FUNC&& h) {
    add_handler(http::verb::get,path,(FUNC&&)h);
  }
  auto& handler_for_verb(http::verb v){
    switch(v){
            case http::verb::get:
            return get_handlers;
            case http::verb::put:
            case http::verb::post:
            return post_handlers;
    }
    return get_handlers;
  }

  auto process_request(auto& req){
    auto httpfunc = bingo::parse_function(to_ssv(req.target()));
    auto& handlers=handler_for_verb(req.method());
    if (auto iter = handlers.find(httpfunc.name()); iter != std::end(handlers)) {
      return handlers[httpfunc.name()]->handle(req,httpfunc);
    }
    throw file_not_found(httpfunc.name());
  }
  std::map<std::string, std::unique_ptr<handler_base>> get_handlers;
  std::map<std::string, std::unique_ptr<handler_base>> post_handlers;
};

} // namespace bingo
