// Server side C/C++ program to demonstrate Socket
// programming
#include "bingo.hpp"
#define PORT 8089

using namespace bingo;

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  try {
    unifex::static_thread_pool context;

    auto server =
        make_listener("127.0.0.1", PORT) |
        process_clients(context,
                        make_request_handler([](Buffer buff) { return buff; }));
    unifex::sync_wait(server);
  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;

  // sock_stream listener;
  // listener.bind({"127.0.0.1",PORT});
  // listen(listener);
  // while(true){
  //     sock_address foriengAddr;
  //     auto incoming = accept(listener);
  //     std::vector<char> vec;vec.reserve(1024);
  //     Buffer buffer{vec.data(),vec.capacity()};
  //     int bytes = read(incoming,buffer);
  //     auto v=buffer.buffer();
  //     send(incoming,buffer);
  // }

  return 0;
}
