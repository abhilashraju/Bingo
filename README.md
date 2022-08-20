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
```

Above example shows a concurrent socket client application. One dedicated thread for socket communication and another for User IO. You can run multiple instance of the client application to see how it handles concurrency effectievely. Depending on the version of the server it connects to the same application behaves just like an echo client or a chat client. 

You can enhance the client application further into an http client library like curl, an MQTT broker client or any other TCP client application.


<center> Fig Client and Server Commnad Line Application </center>

![Chat](/docs/images/example1.PNG)

