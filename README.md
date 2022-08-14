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
                          peer_to_peer_handler([](auto buff) { return buff; })));
    

   
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
                          broadcast_handler([](auto buff) { return buff; })));
                 
  } catch (std::exception &e) {
    printf("%s", e.what());
  }

  return 0;
}
```
Above is an example of broad casting server implementation. This time the server broad cast the message to all connected clients. The above server can be enchanced to write a wide range of application from simple chat service to more complicated queing/messaging protocols such as MQTT or AMQP etc.

### Example For Client Application

```
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
    if (data != std::string()) {
      send(client.sock, string_buffer(data));
      return;
    }
  };

  auto work = unifex::schedule(net_thread.get_scheduler()) |
              unifex::then(wait_for_remote) |
              unifex::then(process_remote_data) |
              unifex::repeat_effect_until([&]() { return status == "error"; });

  auto ui = unifex::schedule(io_thread.get_scheduler()) |
            unifex::then(wait_for_user) | unifex::then(process_user_data) |
            unifex::repeat_effect_until([&]() { return status == "error"; });

  context.spawn(std::move(work));
  context.spawn(std::move(ui));
  context.run();
  return 0;
}
```

Above example shows a concurrent socket client application. One dedicated thread for socket communication and another for User IO. You can run multiple instance of the client application to see how it handles concurrency effectievely. Depending on the version of the server it connects to the same application behaves just like an echo client or a chat client. 

You can enhance the client application further into an http client library like curl, an MQTT broker client or any other TCP client application.


<center> Fig Client and Server Commnad Line Application </center>

![Chat](/docs/images/example1.PNG)

