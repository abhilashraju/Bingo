// Client side C/C++ program to demonstrate Socket
// programming
#include "async_stream.hpp"
#include "io_context.hpp"
#include "unifex/then.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/typed_via.hpp"
#include <iostream>
#include <thread>
int PORT = 8089;

using namespace bingo;
int main(int argc, char const *argv[]) {

  
  io_context context;
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;
  async_sock client;
  connect(client.sock, {"127.0.0.1", PORT});
  auto wait_for_remote = [&]() {
    return client.sync_read(
        [](auto data) { return std::string(data.buffer()); });
  };
  auto process_remote_data = [](auto data) {
      std::cout<<data;
  };
  
  auto wait_for_user=[](){
      return async_io().sync_read( [](auto data) {  return data; });
  };
  auto process_user = [&](auto data) {
      send(client.sock,data);
  };
  
  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(wait_for_remote) |
              unifex::then(process_remote_data) | 
              unifex::repeat_effect();
  auto ui =   unifex::schedule(io_thread.get_scheduler()) |
              unifex::then(wait_for_user) |
              unifex::then(process_user) | 
              unifex::repeat_effect();
              
  context.spawn(std::move(work));
  context.spawn(std::move(ui));
  context.run();
}
