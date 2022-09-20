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

### Example for HTTP Server
```
#define PORT 8080

using namespace bingo;
auto validate_request() {
  return [](auto &buff) {
    std::stringstream stream;
    stream << buff.data();
    beast::flat_buffer buffer_;

    beast::http::request<beast::http::string_body> req_;
    beast::error_code ec{};
    read_istream(stream, buffer_, req_, ec);
    return unifex::just(req_);
  };
}
auto handle_request(auto doc_root) {
  return [doc_root=std::move(doc_root)](auto &&req_) {
    auto var = std::move(handle_request(doc_root, std::move(req_)));

    return unifex::just(std::move(var));
  };
}
auto make_error(bingo::status st, std::string_view error) {
  bingo::response<std::string> res{error.data(), st, 11};
  res.set(bingo::field::server, "bingo:0.0.1");
  res.set(bingo::field::content_type, "text/plain");
  res.set(bingo::field::content_length, std::to_string(error.length()));
  return res;
}
auto error_to_response() {
  return [](auto expn) {
    try {
      std::rethrow_exception(expn);
    } catch (const file_not_found &e) {
      return unifex::just(make_error(bingo::status::not_found,
                                     std::string(e.what()) + " Not Found"));
    } catch (const std::invalid_argument &e) {
      return unifex::just(
          make_error(bingo::status::forbidden,
                     std::string(e.what()) + "Invalid Argument"));
    } catch (const std::exception &e) {
      return unifex::just(make_error(bingo::status::internal_server_error,
                                     std::string(e.what()) + " Server Error"));
    }
  };
}
auto let_stopped() {
  return []() { return unifex::just(std::string("Stopped")); };
}
auto send_response() {
  return [](auto &res) { return unifex::just(serialize(res)); };
}
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  namespace fs = std::filesystem;
  std::string doc_root=fs::current_path().c_str();
  if(argc >1){
    doc_root=argv[1];
  }
  unifex::static_thread_pool context;
  unifex::inplace_stop_source stop_src;

  auto http_worker = [=](auto buff) {
    return unifex::just(buff) 
          | unifex::let_value(validate_request()) 
          | unifex::let_value(handle_request(doc_root)) 
          | unifex::let_error(error_to_response()) 
          | unifex::let_value(send_response());
  };
  make_listener("127.0.0.1", PORT) |
      listen_for_peer_to_peer_connection(
          context.get_scheduler(), stop_src.get_token(),
          context.get_scheduler(), std::move(http_worker)) |
      handle_error([&](std::exception &v) {
        stop_src.request_stop();
        printf("%s", v.what());
      }) |
      unifex::sync_wait();

  return 0;
}
```
This is a simple http server example. Look at how the request are handled.
```
auto http_worker = [=](auto buff) {
    return unifex::just(buff) 
          | unifex::let_value(validate_request()) 
          | unifex::let_value(handle_request(doc_root)) 
          | unifex::let_error(error_to_response()) 
          | unifex::let_value(send_response());
  };

``` 
There is separate channel for value and error handling. While the handle_request will do the  vallid response generation , the error_to_response function collects all the error scenarios occured and creates an error repsonse. Separating value and error channel will simplify the server implementation and customization.

Also note that the main server logic is not concerned about the scheduling issues. The concern is properly seprated and handled by the choice of scheduler . Above example uses static thread pool to do it. 

### Example Web Server Application
```
#include "web_server.hpp"
#include <filesystem>
#include <iostream>
#define PORT 8080

using namespace bingo;

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  namespace fs = std::filesystem;
  std::string doc_root = fs::current_path().c_str();
  if (argc > 1) {
    doc_root = argv[1];
  }
  web_server server;
  auto plain_text_handler= [](auto func){
    return [func=std::move(func)](auto& req,auto& httpfunc){
      http::response<http::string_body> resp{http::status::ok, req.version()};
      resp.set(http::field::content_type, "text/plain");
      resp.body()=func(req,httpfunc);
      resp.set(http::field::content_length, std::to_string(resp.body().length()));
      return resp;
    };
  };
  server.add_get_handler("/hello",[](auto& req,auto& httpfunc){
    http::response<http::string_body> resp{http::status::ok, req.version()};
    resp.set(http::field::content_type, "text/plain");
    resp.body()="hello world";
    resp.set(http::field::content_length, std::to_string(resp.body().length()));
    return resp;

  });

  server.add_get_handler("/greetings",plain_text_handler([](auto& req,auto& httpfunc){
    return "Hello " + httpfunc["name"] +"!!!";
  }));
  server.start(doc_root,PORT);

  return 0;
}
```
The web server application is a higher level abstraction over the http server. The idea is to mimic Spring boot Web application for java using C++. The web server mimics the  @RequestMapping annotation to map path to function handler but using C++. 


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

