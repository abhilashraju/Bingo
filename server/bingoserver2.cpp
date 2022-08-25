// Server side C/C++ program to demonstrate unifex based chat server
// programming
#include "bingo.hpp"

#define PORT 8089

using namespace bingo;

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;

  unifex::static_thread_pool context;
  unifex::inplace_stop_source stop_src;
  int count=0;
  auto worker = [](auto buff) {
    std::string newbuf;
    newbuf.resize(buff.read_length());
    std::copy(buff.data(), buff.data() + buff.read_length(), newbuf.data());
    if(buff.data() ==std::string("throw\n"))
      throw application_error("application error");
    return unifex::just(std::move(newbuf));
  };
  make_listener("127.0.0.1", PORT) |
      listen_for_peer_to_peer_connection(
          context.get_scheduler(), stop_src.get_token(),
          context.get_scheduler(), std::move(worker)) |
      handle_error([&](std::exception &v) {
        stop_src.request_stop();
        printf("%s", v.what());
      }) |
      unifex::sync_wait();

  return 0;
}
