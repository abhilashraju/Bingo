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

int main(int argc, char const *argv[]) {
  using namespace bingo;
  io_context context;
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;

  async_sock client;
  connect(client.sock, {"127.0.0.1", PORT});

  thread_data remotedata;
  async_io io;
  thread_data io_data;
  auto clean_up = [&]() {
    remotedata.request_stop();
    io_data.request_stop();
    context.request_stop();
  };
  auto work =
      unifex::schedule(net_thread.get_scheduler()) |
      wait_for_io(client, remotedata.get_buffer()) | unifex::then([&](auto) {
        std::cout << remotedata.get_buffer().data();
        remotedata.get_buffer().consume_all();
      }) |
      unifex::repeat_effect_until([token = remotedata.get_token()]() {
        return token.stop_requested();
      }) |
      handle_error(
          [&](std::exception &e) {
            std::cout << "Server Error \n";
            clean_up();
          },
          [&](std::string &e) { std::cout << "Server Error " << e << "\n"; });

  auto ui = unifex::schedule(io_thread.get_scheduler()) |
            wait_for_io(io, io_data.get_buffer()) | unifex::then([&](auto) {
              send(client.sock, io_data.get_buffer());
              io_data.get_buffer().consume_all();
            }) |
            unifex::repeat_effect_until([token = io_data.get_token()]() {
              return token.stop_requested();
            }) |
            handle_error([&](auto &e) {
              std::cout << "Client Error\n";
              clean_up();
            });
  context.spawn_all(std::move(work), std::move(ui));
  context.run();
  return 0;
}
