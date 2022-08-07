# Bingo
A Generic Server and Client frame work  using sender receiver pattern

This library is intented to be used as a base frame work for developing Server and Client applications. This is an experimental implementation of C++ [sender/reciever](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0443r14.html) asynchronous programming model. This uses [libunifex](https://github.com/facebookexperimental/libunifex/blob/main/doc/overview.md) as the base for reusable sender/reciever algorithm. There is no external third party dependecy other than the [libunifex](https://github.com/facebookexperimental/libunifex/blob/main/doc/overview.md) library. 

### Example for Peer to Peer Server
```
#include "bingo.hpp"
#define PORT 8089

using namespace bingo;

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  
  try {
    unifex::static_thread_pool context;

 
      unifex::sync_wait(
          make_listener("127.0.0.1", PORT) |
          process_clients(context,
                          peer_to_peer_handler([](Buffer buff) { return buff; })));
    

   
  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;
}
```
Above is an example server side implementation of a peer to peer communication. The sever just act as an echo server handling multiple client at time. This serve can be used as base frame work for wide range of applications such as simple echo server to more complicated html or other kind of webservers.

### Example for broad casting Server
```
#include "bingo.hpp"
#define PORT 8089

using namespace bingo;

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  
  try {
    unifex::static_thread_pool context;

 
      unifex::sync_wait(
          make_listener("127.0.0.1", PORT) |
          process_clients(context,
                          broadcast_handler([](Buffer buff) { return buff; })));
                 
  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;
}
```
Above is an example of broad casting server implementation. This time the server broad cast the message to all connected clients. The above server can be enchanced to write a wide range of application from simple chat service to more complicated queing/messaging protocols such as MQTT or AMQP etc.