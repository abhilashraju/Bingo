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
  async_sock client;
  connect(client, {"127.0.0.1", PORT});

  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;
  auto waitforNetdata = [&]() {
    return on_read(client,
                   [](auto data) { return std::string(data.buffer()); });
  };
  auto process = [&](auto data) {
    std::cout << data;
    std::string userdata;
    std::cin >> userdata;
    send(client.sock, Buffer(userdata));
  };

  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(waitforNetdata) |
              unifex::typed_via(io_thread.get_scheduler()) |
              unifex::then(process) | unifex::repeat_effect();
  std::string start("Start");
  send(client.sock, Buffer(start));
  unifex::sync_wait(std::move(work));
}
