#include "bingosock.hpp"
#include "reactor.hpp"
#include <sys/select.h>
#include <unifex/async_scope.hpp>
#include <unifex/just.hpp>
#include <unifex/let_error.hpp>
#include <unifex/let_value.hpp>
#include <unifex/on.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>
namespace bingo {

auto make_listener(auto address, auto port) {
  return unifex::just_from([=]() {
    sock_stream *listener = new sock_stream;
    listener->bind({address, port});
    listen(*listener);
    return listener;
  });
}
template <typename Context, typename Handler> struct handle_clients {
  Context &context;
  Handler handler;
  handle_clients(Context &C, Handler h) : context(C), handler(std::move(h)) {}
  auto get() const {
    return unifex::then([=](auto l) {
      std::unique_ptr<sock_stream> listener(l);
      unifex::async_scope scope;
      Reactor reactor(
          [&](sock_stream newconnection) {
            handler.spawn(scope, context, std::move(newconnection));
          },
          [&](int fd) { return handler.handle_read(fd); });
      reactor.run(*listener);

      return std::string("Server Started");
    });
  }
  template <typename Sender, typename C, typename H>
  friend auto operator|(Sender &&sender, handle_clients<C, H> &&clihandler) {
    return sender | clihandler.get();
  }
};
template <typename Context, typename H>
auto process_clients(Context &where, H handler) {
  return handle_clients(where, std::move(handler));
}

template <typename Handler> struct peer_to_peer_handler {
  using Request_Handler = Handler;
  Request_Handler request_handler;
  static constexpr bool broad_casting = false;
  peer_to_peer_handler(Request_Handler handler)
      : request_handler(std::move(handler)) {}
  auto spawn(auto &scope, auto &context, auto newconnection) const {
    scope.spawn(unifex::on(context.get_scheduler(),
                           handleConnection(std::move(newconnection))));
  }
  auto handle_read(int fd) const { return false; }
  auto handleConnection(sock_stream newsock) const {
    auto newclient = unifex::just(new sock_stream(std::move(newsock)));
    auto responder = unifex::then([=](auto newsock) {
      std::unique_ptr<sock_stream> sock(newsock);
      try {
        while (true) {
          std::vector<char> vec;
          vec.reserve(1024);
          Buffer buffer{vec.data(), vec.capacity()};
          read(*sock, buffer);
          send(*sock, request_handler(buffer));
        }
      } catch (std::exception &e) {
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
template <typename Handler> struct broadcast_handler {
  using Request_Handler = Handler;
  Request_Handler request_handler;
  static constexpr bool broad_casting = true;
  struct ClientList {
    std::vector<std::unique_ptr<sock_stream>> clients;
    std::mutex client_mutex;
    void broadcast(Buffer buffer) {
      std::lock_guard<std::mutex> guard(client_mutex);
      for (auto &c : clients) {
        send(*c, buffer);
      }
    }
    void add_client(sock_stream *client) {
      std::lock_guard<std::mutex> guard(client_mutex);
      clients.emplace_back(client);
    }
    void remove_client(sock_stream *client) {
      std::lock_guard<std::mutex> guard(client_mutex);
      clients.erase(std::remove_if(std::begin(clients), std::end(clients),
                                   [&](auto &e) { return e.get() == client; }));
    }
    std::optional<sock_stream *> find(int fd) {
      std::lock_guard<std::mutex> guard(client_mutex);
      auto const& iter = std::find_if(cbegin(clients), cend(clients),
                                [=](const auto &sock) { return sock->fd_ == fd; });
      if (iter != cend(clients)) {
        return std::optional(iter->get());
      }
      return std::nullopt;
    }
  };
  static auto &getClientList() {
    static ClientList gclient_lists;
    return gclient_lists;
  }

  broadcast_handler(Request_Handler handler)
      : request_handler(std::move(handler)) {}
  auto handleConnection(sock_stream newsock) const {
    auto newclient = unifex::just(new sock_stream(std::move(newsock)));
    auto responder = unifex::then([=](auto newsock) {
      getClientList().add_client(newsock);
      try {
        while (true) {
          std::vector<char> vec;
          vec.reserve(1024);
          Buffer buffer{vec.data(), vec.capacity()};
          read(*newsock, buffer);
          getClientList().broadcast(request_handler(buffer));
        }
      } catch (std::exception &e) {
        getClientList().remove_client(newsock);
      }
    });
    auto session = newclient | responder;
    return session;
  }
  auto spawn(auto &scope, auto &context, auto newsock) const {
    getClientList().add_client(new sock_stream(std::move(newsock)));
  }
  auto handle_read(int fd) const {
    auto sock = getClientList().find(fd);
    if (sock) {
      std::vector<char> vec;
      vec.reserve(1024);
      Buffer buffer{vec.data(), vec.capacity()};
      auto n = read(*sock.value(), buffer);
      if(n<=0){
        getClientList().remove_client(sock.value());
        return false;//socket closed by remote
      }
       getClientList().broadcast(request_handler(buffer));
    }
    return true;
  }
};

inline auto error_to_response(std::exception_ptr err) {
  try {
    std::rethrow_exception(err);
  } catch (const std::exception &e) {
    return unifex::just(std::string(e.what()));
  }
}
template <typename Func> struct upon_error {
  Func callback;
  upon_error(Func func) : callback(std::move(func)) {}
  template <typename Sender>
  friend auto operator|(Sender sender, upon_error onerror) {
    return sender | unifex::let_error(error_to_response) |
           unifex::then([onerror = std::move(onerror)](auto v) {
             return onerror.callback(v);
           });
  }
};
struct EchoHandler {
  Buffer operator()(Buffer buff) const { return buff; }
};
} // namespace bingo