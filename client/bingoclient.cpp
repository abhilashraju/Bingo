// Client side C/C++ program to demonstrate Socket
// programming
#include "async_sock.hpp"
#include "bingo.hpp"
#include "unifex/inline_scheduler.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/typed_via.hpp"
#include <iostream>
#include <thread>
int PORT = 8089;

using namespace bingo;
int main(int argc, char const *argv[]) {

  
  unifex::async_scope scope;
  unifex::single_thread_context main_thread;
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;
  async_sock client;
  connect(client.sock, {"127.0.0.1", PORT});
  auto waitforNetdata = [&]() {
    return client.sync_read(
        [](auto data) { return std::string(data.buffer()); });
  };
  auto process = [&](auto data) mutable {
      std::cout<<data;
  };
  
  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(waitforNetdata) |
              unifex::then(process) | unifex::repeat_effect();
  auto ui =   unifex::schedule(io_thread.get_scheduler()) |
              unifex::then([](){return async_io().sync_read( [](auto data) { return data; });}) |
              unifex::then([&](auto buffer){send(client.sock,buffer);}) | unifex::repeat_effect();
 
  scope.spawn(unifex::on(main_thread.get_scheduler(), work));
  scope.spawn(unifex::on(main_thread.get_scheduler(), ui));

  GenericReactor::get_reactor().run();
}
