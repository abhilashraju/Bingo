#pragma once
#include "reactor.hpp"
#include "unifex/async_scope.hpp"
#include "unifex/inline_scheduler.hpp"
#include "unifex/inplace_stop_token.hpp"
namespace bingo {
struct io_context {
  unifex::async_scope scope;
  unifex::inline_scheduler main_thread;
  unifex::inplace_stop_source stop_src;
  template <typename work> void spawn(work &&w) {
    scope.spawn(unifex::on(main_thread, std::forward<work>(w)));
  }
  template <typename... Senders> void spawn_all(Senders... senders) {

    (scope.spawn(unifex::on(main_thread, std::forward<Senders>(senders))), ...);
  }
  void run() { GenericReactor::get_reactor().run(stop_src.get_token()); }
  auto get_token() { return stop_src.get_token(); }
  auto request_stop() { scope.request_stop();return stop_src.request_stop(); }
};

struct thread_data {
  unifex::inplace_stop_source remote_stop_src;
  std::string data;
  string_buffer buff{data};
  auto &get_buffer() { return buff; }
  auto get_token() { return remote_stop_src.get_token(); }
  void request_stop() { remote_stop_src.request_stop(); }
};
} // namespace bingo
