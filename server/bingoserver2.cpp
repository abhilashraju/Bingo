// Server side C/C++ program to demonstrate unifex based chat server
// programming
#include "bingo.hpp"
#include "unifex/inplace_stop_token.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/typed_via.hpp"
#define PORT 8089

using namespace bingo;

inline auto read_data(auto &stream, auto &buff) {

  if (int n; n = read(stream, buff) > 0) {
    printf("%s", buff.data());
    return n;
  }
  throw std::runtime_error("client closed");
}
inline auto write_data(auto &stream,auto& buff) {
  if (int n; n = send(stream, buff) > 0) {
    buff.consume(n);
  }
 
}
auto spawn_clients(auto sched, auto newconnection) {
  struct async_context {
    sock_stream stream;
    unifex::async_scope scope;
    unifex::inplace_stop_source stop_src;
    std::string v;
    string_buffer buff{v};
    unifex::inplace_stop_token token{stop_src.get_token()};
    async_context(sock_stream conn) : stream(std::move(conn)) {}
  };
  auto context = new async_context(std::move(newconnection));
  auto work = unifex::just_from(
                  [=]() { return std::unique_ptr<async_context>(context); }) |
              unifex::let_value([](auto &context) {
                auto work =
                    unifex::just_from([contextptr = context.get()]() {
                      return read_data(contextptr->stream, contextptr->buff);
                    }) |
                    unifex::then([contextptr = context.get()](auto len) {
                      return write_data(contextptr->stream, contextptr->buff);
                    }) |
                    unifex::repeat_effect_until([contextptr = context.get()]() {
                      return contextptr->token.stop_requested();
                    }) |
                    handle_error([contextptr = context.get()](auto &v) {
                      contextptr->stop_src.request_stop();
                    });
                return work;
              });

  context->scope.spawn_on(sched, work);
}
inline auto listen(auto agent, auto token, auto client_agent) {
  return unifex::let_value([=](auto &listener) {
    return unifex::just(listener) | unifex::typed_via(agent) |
           unifex::then([](auto listener) {
             auto newsock = accept(*listener);
             return newsock;
           }) |
           unifex::then([=](auto newconn) {
             return spawn_clients(client_agent, std::move(newconn));
           }) |
           unifex::repeat_effect_until(
               [=]() { return token.stop_requested(); });
  });
}
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;

  try {
    unifex::static_thread_pool context;
    unifex::inplace_stop_source strop_src;

    unifex::sync_wait(make_listener("127.0.0.1", PORT) |
                      listen(context.get_scheduler(), strop_src.get_token(),
                             context.get_scheduler()));

  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;
}
