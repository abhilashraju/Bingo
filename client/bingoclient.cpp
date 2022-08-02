// Client side C/C++ program to demonstrate Socket
// programming
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

  sock_stream client;
  auto newstream = connect(client, {"127.0.0.1", PORT});
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;

  auto getInput = []() {
    std::string str;
    std::cin >> str;
    return str;
  };
  auto sendToServer = [&](auto tosend) {
    send(client, Buffer(tosend));
    std::array<char, 1024> arry{0};
    Buffer buffer{arry.data(), arry.size()};
    int bytes = read(client, buffer);
    auto v = buffer.buffer();
    std::printf("%s\n", v);
  };

  auto work = unifex::schedule(io_thread.get_scheduler()) |
              unifex::then(getInput) |
              unifex::typed_via(net_thread.get_scheduler()) |
              unifex::then(sendToServer) | unifex::repeat_effect();

  unifex::sync_wait(std::move(work));
}
