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
                        broadcast_handler([](Buffer buff) { return buff; }));
    unifex::sync_wait(server);
  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;
}
