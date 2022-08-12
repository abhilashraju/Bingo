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
  std::string status = "running";
  auto wait_for_remote = [&]() {
    try {
      return client.sync_read(
          [](auto data) { return std::string(data.data()); });
    } catch (std::exception e) {
      status = "error";
      return std::string(e.what());
    }
  };
  auto process_remote_data = [](auto data) { std::cout << data; };

  auto wait_for_user = [&]() {
    try {
      return async_io().sync_read(
          [=](auto data) { return Greeting + data.data(); });
    } catch (std::exception e) {
      status = "error";
      return std::string();
    }
  };
  auto process_user_data = [&](auto data) {
    if(data!=std::string()){
       send(client.sock, buffer(data));
       return;
    }
  };

  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(wait_for_remote) |
              unifex::then(process_remote_data) |
              unifex::repeat_effect_until([&]() { return status == "error"; });

  auto ui = unifex::schedule(io_thread.get_scheduler()) |
            unifex::then(wait_for_user) | 
            unifex::then(process_user_data) |
            unifex::repeat_effect_until([&]() { return status == "error"; });

  context.spawn(std::move(work));
  context.spawn(std::move(ui));
  context.run();
  return 0;
}
