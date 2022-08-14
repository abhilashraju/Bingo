// Client side C/C++ program to demonstrate Sender/Reciever Based Sockets
// programming
#include "async_stream.hpp"
#include "io_context.hpp"
#include "unifex/let_error.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/then.hpp"
#include "unifex/when_all.hpp"

#include <iostream>
#include <sstream>
#include <thread>
int PORT = 8089;

using namespace bingo;
enum class _state { running, error };
int main(int argc, char const *argv[]) {

  auto Greeting = [&]() {
    if (argc > 1) {
      return std::string(argv[1]);
    }
    std::stringstream strstream;
    strstream << std::this_thread::get_id();
    return strstream.str();
  }() + " Says: ";
  io_context context;
  unifex::single_thread_context io_thread;
  unifex::single_thread_context net_thread;
  async_sock client;
  connect(client.sock, {"127.0.0.1", PORT});

  std::atomic<_state> status{_state::running};
  auto secure_handler = [&](auto handle) {
    try {
      return handle();
    } catch (std::exception &e) {
      status.exchange(_state::error);
      context.request_stop();
      return std::string(e.what());
    }
  };
  auto wait_for_remote = [&]() {
    return secure_handler([&](){
      return client.sync_read(
          [](auto data) { return std::string(data.data()); });
    });
  };
  auto process_remote_data = [](auto data) { std::cout << data; };

  auto wait_for_user = [&]() {
    return secure_handler([&](){
         return async_io().sync_read(
          [=](auto data) { return Greeting + data.data(); });
    });
  };
  auto process_user_data = [&](auto data) {
    if (data != std::string()) {
      send(client.sock, string_buffer(data));
      return;
    }
  };

  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(wait_for_remote) |
              unifex::then(process_remote_data) |
              unifex::repeat_effect_until(
                  [&]() { return status.load() == _state::error; });

  auto ui = unifex::schedule(io_thread.get_scheduler()) |
            unifex::then(wait_for_user) | unifex::then(process_user_data) |
            unifex::repeat_effect_until(
                [&]() { return status.load() == _state::error; });

  context.spawn(std::move(work));
  context.spawn(std::move(ui));
  context.run();
  return 0;
}
