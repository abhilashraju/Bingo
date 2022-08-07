#pragma once
#include "unifex/async_scope.hpp"
#include "unifex/inline_scheduler.hpp"
#include "reactor.hpp"
namespace bingo{
struct io_context{
  unifex::async_scope scope;
  unifex::inline_scheduler main_thread;
  template<typename work>
  void spawn(work&& w){
     scope.spawn(unifex::on(main_thread, std::forward<work>(w)));
  }
  void run(){
    GenericReactor::get_reactor().run();
  }
};
}
