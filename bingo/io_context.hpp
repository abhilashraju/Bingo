#pragma once
#include "unifex/async_scope.hpp"
#include "unifex/inline_scheduler.hpp"
#include "unifex/inplace_stop_token.hpp"
#include "reactor.hpp"
namespace bingo{
struct io_context{
  unifex::async_scope scope;
  unifex::inline_scheduler main_thread;
  unifex::inplace_stop_source stop_src;
  template<typename work>
  void spawn(work&& w){
     scope.spawn(unifex::on(main_thread, std::forward<work>(w)));
  }
  void run(){
    GenericReactor::get_reactor().run(stop_src.get_token());
  }
  auto get_token(){
    return stop_src.get_token();
  }
  auto request_stop(){
    return stop_src.request_stop();
  }
};
}
