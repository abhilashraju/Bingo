// Client side C/C++ program to demonstrate Sender/Reciever Based Sockets
// programming
#include "async_stream.hpp"
#include "io_context.hpp"
#include "unifex/let_error.hpp"
#include "unifex/let_value.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/single_thread_context.hpp"
#include "unifex/then.hpp"
#include "unifex/when_all.hpp"
#include <iostream>
#include <sstream>
#include <thread>
int PORT = 8089;

using namespace bingo;

template <typename Stream,typename Handler> struct data_processor {

  Stream &stream;
  unifex::inplace_stop_source stop_src;
  Handler handler;
  std::vector<char> v;
  vector_buffer buf{v};
  data_processor(Stream &s, Handler h) : stream(s), handler(std::move(h)) {}
  void request_stop() { stop_src.request_stop(); }
  
  friend auto operator|(auto sender, data_processor &processor) {

    auto wait_for_remote = [&]() {
      try {
        return processor.stream.sync_read(processor.buf,
                                        [&]() { return processor.buf.read_view(); });
      } catch (std::exception &e) {
        processor.buf.consume_all();
        auto len =strlen(e.what());
        std::copy_n(e.what(),len,processor.buf.prepare(len));
        processor.buf.commit(len);
        processor.stop_src.request_stop();
        return processor.buf.read_view();
      }
    };
    auto readSender = sender | unifex::then(wait_for_remote);
    auto processingSender = processor.handler(readSender);
    return processingSender | unifex::then([&]() {
             processor.buf.consume_all();
           }) |
           unifex::repeat_effect_until([&]() {
             return processor.stop_src.get_token().stop_requested();
           });
  }
};

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

 
  auto token = context.get_token();
  auto remote_processor = data_processor(client, [](auto &sender) {
    return sender | unifex::then([](auto data) {
             std::cout << data.data();
             data.consume(data.read_length());
           });
  });

  async_io io;
  auto user_processor = data_processor(io, [&](auto &sender) {
    return sender | unifex::then([&](auto buff) { send(client.sock, buff);});
  });

  auto work = unifex::schedule(net_thread.get_scheduler()) | remote_processor;

  auto ui =
      unifex::schedule(io_thread.get_scheduler()) | user_processor;

  context.spawn(std::move(work));
  context.spawn(std::move(ui));
  context.run();
  return 0;
}
