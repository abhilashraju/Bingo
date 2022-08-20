// Client side C/C++ program to demonstrate Sender/Reciever Based Sockets
// programming
#include "async_stream.hpp"
#include "io_context.hpp"
#include "stream_processor.hpp"
#include "unifex/just_error.hpp"
#include "unifex/let_error.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/upon_error.hpp"
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
  auto handle_error = [](auto... handlers) {
    return [=](auto exptr) {
      try {
        std::rethrow_exception(exptr);
      } catch (const std::exception &e) {
        (handlers(),...);
      }
    };
  };

  auto remote_processor = stream_processor(
      client,
      [](auto sender) {
        return sender |
               unifex::then([](auto data) { std::cout << data.data(); });
      },
      handle_error([](){std::cout<<"server errror\n";}
                  ,[&](){context.request_stop();}));

  async_io io;
  auto user_processor = stream_processor(
      io,
      [&](auto sender) {
        return sender |
               unifex::then([&](auto buff) { send(client.sock, buff); });
      },
      handle_error([](){std::cout<<"client errror\n";}
                  ,[&](){context.request_stop();}));

  auto work = unifex::schedule(net_thread.get_scheduler()) | remote_processor;

  auto ui = unifex::schedule(io_thread.get_scheduler()) | user_processor;

  context.spawn_all(std::move(work), std::move(ui));
  context.run();
  return 0;
}
