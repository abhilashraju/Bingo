#pragma once
#include "bingosock.hpp"
#include "buffer.hpp"
#include "errors.hpp"
#include "reactor.hpp"
#include "unifex/inplace_stop_token.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/typed_via.hpp"
#include <sys/select.h>
#include <unifex/async_scope.hpp>
#include <unifex/just.hpp>
#include <unifex/just_error.hpp>
#include <unifex/let_error.hpp>
#include <unifex/let_value.hpp>
#include <unifex/on.hpp>
#include <unifex/retry_when.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>
#include <unifex/upon_error.hpp>
#include <variant>
namespace bingo {

inline auto make_listener(auto address, auto port) {
  return unifex::just_from([=]() {
    sock_stream* listener = new sock_stream;

    int retries = 0;
    bool connected = false;
    while (!connected && retries < 5) {
      try {
        listener->bind({address, port});
        listen(*listener);
        connected = true;
      } catch (std::exception& e) {
        retries++;
        sleep(10);
      }
    }
    if (!connected) {
      throw std::runtime_error("Not Able to listen . Port may be in use...\n");
    }
    return listener;
  });
}

template <typename Context, typename Handler>
struct handle_clients {
  Context& context;
  Handler handler;
  handle_clients(Context& C, Handler h) : context(C), handler(std::move(h)) {}
  auto get() const {
    return unifex::then([=](auto l) {
      std::unique_ptr<sock_stream> listener(l);
      unifex::async_scope scope;
      ConnectionReactor reactor(
          [&](sock_stream newconnection) {
            handler.spawn(scope, context, std::move(newconnection));
          },
          [&](int fd) { return handler.handle_read(fd); });
      reactor.run(*listener);

      return std::string("Server Started");
    });
  }
  template <typename Sender>
  inline friend auto operator|(Sender&& sender, handle_clients&& clihandler) {
    return sender | clihandler.get();
  }
};
template <typename Context, typename H>
inline auto process_clients(Context& where, H handler) {
  return handle_clients(where, std::move(handler));
}

template <typename Handler>
struct peer_to_peer_handler {
  using Request_Handler = Handler;
  Request_Handler request_handler;
  static constexpr bool broad_casting = false;
  peer_to_peer_handler(Request_Handler handler)
    : request_handler(std::move(handler)) {}
  auto spawn(auto& scope, auto& context, auto newconnection) const {
    scope.spawn(unifex::on(
        context.get_scheduler(), handleConnection(std::move(newconnection))));
  }
  auto handle_read(int fd) const { return false; }
  auto handleConnection(sock_stream newsock) const {
    auto newclient = unifex::just(new sock_stream(std::move(newsock)));
    auto responder = unifex::then([=](auto newsock) {
      std::unique_ptr<sock_stream> sock(newsock);
      try {
        while (true) {
          std::string str;
          string_buffer buf(str);
          int n = read(*sock, buf);
          if (n == 0) {
            throw socket_exception(std::string("EOF"));
          }
          send(*sock, request_handler(buf));
        }
      } catch (std::exception& e) {
        printf("%s", e.what());
      }
    });
    auto session = newclient | responder;
    return session;
  }
};
template <typename T>
inline peer_to_peer_handler<T> make_peer_to_peer_handler(T handler) {
  return peer_to_peer_handler<T>(handler);
}
template <typename Handler>
struct broadcast_handler {
  using Request_Handler = Handler;
  Request_Handler request_handler;
  static constexpr bool broad_casting = true;
  struct ClientList {
    std::vector<std::unique_ptr<sock_stream>> clients;
    std::mutex client_mutex;
    void broadcast(auto buf) {
      std::lock_guard<std::mutex> guard(client_mutex);
      for (auto& c : clients) {
        send(*c, buf);
      }
    }
    void add_client(sock_stream* client) {
      std::lock_guard<std::mutex> guard(client_mutex);
      clients.emplace_back(client);
    }
    void remove_client(sock_stream* client) {
      std::lock_guard<std::mutex> guard(client_mutex);
      clients.erase(
          std::remove_if(std::begin(clients), std::end(clients), [&](auto& e) {
            return e.get() == client;
          }));
    }
    std::optional<sock_stream*> find(int fd) {
      std::lock_guard<std::mutex> guard(client_mutex);
      auto const& iter =
          std::find_if(cbegin(clients), cend(clients), [=](const auto& sock) {
            return sock->fd_ == fd;
          });
      if (iter != cend(clients)) {
        return std::optional(iter->get());
      }
      return std::nullopt;
    }
  };
  static auto& getClientList() {
    static ClientList gclient_lists;
    return gclient_lists;
  }

  broadcast_handler(Request_Handler handler)
    : request_handler(std::move(handler)) {}

  auto spawn(auto& scope, auto& context, auto newsock) const {
    getClientList().add_client(new sock_stream(std::move(newsock)));
  }
  auto handle_read(int fd) const {
    auto sock = getClientList().find(fd);
    if (sock) {
      std::string str;
      string_buffer buf(str);
      auto n = read(*sock.value(), buf);
      if (n <= 0) {
        getClientList().remove_client(sock.value());
        return false;  // socket closed by remote
      }
      getClientList().broadcast(request_handler(buf));
    }
    return true;
  }
};

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
inline auto handle_error(auto... handlers) {
  return unifex::let_error([=](auto exptr) {
    try {
      std::rethrow_exception(exptr);
    } catch (const std::exception& e) {
      std::variant<std::exception> v(e);
      std::visit(overloaded{handlers...}, v);
    }
    return unifex::just();
  });
}

inline auto read_data(auto& stream, auto& buff) {
  if (int n; (n = read(stream, buff)) > 0) {
    printf("%s", buff.data());
    return n;
  }
  throw socket_exception("client closed");
}
inline auto write_data(auto& stream, auto& buff) {
  if (int n; (n = send(stream, buff)) > 0) {
    buff.consume(n);
  }
}

inline auto spawn_clients(auto client_agent, auto newconnection, auto do_work) {
  struct async_context {
    ~async_context(){
      unifex::sync_wait(scope.cleanup());
    }
    sock_stream stream;
    unifex::async_scope scope;
    unifex::inplace_stop_source stop_src;
    std::string v;
    string_buffer buff{v};
    unifex::inplace_stop_token token{stop_src.get_token()};
    async_context(sock_stream conn) : stream(std::move(conn)) {}
  };
  auto context = new async_context(std::move(newconnection));
  auto work =
      unifex::just_from(
          [=]() { return std::unique_ptr<async_context>(context); }) |
      unifex::let_value([=](auto& context) {
        auto child_work =
            unifex::just_from([contextptr = context.get()]() {
              return read_data(contextptr->stream, contextptr->buff);
            }) |
            unifex::let_value([=, contextptr = context.get()](auto& len) {
              auto client_work =
                  do_work(contextptr->buff.read_view(), contextptr->stop_src);
              return client_work;
            }) |
            unifex::let_value([contextptr = context.get()](auto& buff) {
              string_buffer strbuff(buff);
              write_data(contextptr->stream, strbuff);
              contextptr->buff.consume_all();
              return unifex::just();
            }) |
            unifex::repeat_effect_until([contextptr = context.get()]() {
              if (contextptr->token.stop_requested()) {
                close(contextptr->stream);
              }
              return contextptr->token.stop_requested();
            }) |
            unifex::retry_when([](std::exception_ptr ex) mutable {
              try {
                std::rethrow_exception(ex);
              } catch (application_error& error) {
              }
              return unifex::just();
            }) |
            handle_error([contextptr = context.get()](auto& v) {
              //    contextptr->stop_src.request_stop();
              printf("client closed\n");
              close(contextptr->stream);
              unifex::sync_wait(contextptr->scope.cleanup());
            });
        return child_work;
      });

  context->scope.spawn_on(client_agent, work);
}
inline auto acceptor(auto listener_agent) {
  return unifex::typed_via(listener_agent) | unifex::then([](auto listener) {
           auto newsock = accept(*listener);
           return newsock;
         });
}
inline auto peer_to_peer_sender(auto agent, auto worker) {
  return unifex::then([=, worker = std::move(worker)](auto newconn) {
    return spawn_clients(agent, std::move(newconn), std::move(worker));
  });
}

inline auto listen_for_peer_to_peer_connection(
    auto agent, auto token, auto client_agent, auto worker) {
  return unifex::let_value([=](auto& listener) {
    return unifex::just(listener) | acceptor(agent) |
        peer_to_peer_sender(client_agent, std::move(worker)) |
        unifex::repeat_effect_until([=]() { return token.stop_requested(); });
  });
}
}  // namespace bingo
