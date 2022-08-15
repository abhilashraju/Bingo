// Client side C/C++ program to demonstrate Sender/Reciever Based Sockets
// programming
#include "async_stream.hpp"
#include "io_context.hpp"
#include "stream_processor.hpp"
#include "unifex/single_thread_context.hpp"

#include <iostream>
#include <sstream>
#include <thread>
int PORT = 8089;

using namespace bingo;

int main(int argc, char const *argv[]) {

  io_context context;
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;
  async_sock client;
  connect(client.sock, {"127.0.0.1", PORT});

  auto remote_processor = stream_processor(client, [](auto &sender) {
    return sender | unifex::then([](auto data) { std::cout << data.data(); });
  });

  async_io io;
  auto user_processor = stream_processor(io, [&](auto &sender) {
    return sender | unifex::then([&](auto buff) { send(client.sock, buff); });
  });

  auto work = unifex::schedule(net_thread.get_scheduler()) | remote_processor;

  auto ui = unifex::schedule(io_thread.get_scheduler()) | user_processor;

  context.spawn_all(std::move(work), std::move(ui));
  context.run();
  return 0;
}
