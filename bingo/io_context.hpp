#pragma once
#include "unifex/async_scope.hpp"
#include "unifex/single_thread_context.hpp"
#include "reactor.hpp"
namespace bingo{
struct io_context{
  unifex::async_scope scope;
  unifex::single_thread_context main_thread;
  template<typename work>
  void spawn(work&& w){
     scope.spawn(unifex::on(main_thread.get_scheduler(), std::forward<work>(w)));
  }
  void run(){
    GenericReactor::get_reactor().run();
  }
};
}
