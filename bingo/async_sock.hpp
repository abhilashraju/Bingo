#pragma once
#include "bingosock.hpp"
#include "reactor.hpp"
namespace bingo {
struct async_sock {
  sock_stream sock;
  friend void connect(async_sock &stream, const sock_address &adress) {
    connect(stream.sock, adress);
  }
  template <typename Handler>
  friend auto on_read(async_sock &stream, Handler handler) {
    std::array<char, 1024> arry{0};
    Buffer buffer{arry.data(), arry.size()};
    SocketReactor(stream.sock, [&](int fd) {
      if (fd == stream.sock.fd_) {

        int bytes = read(stream.sock, buffer);
        auto v = buffer.buffer();
        return true;
      }
    }).run();
    return handler(buffer);
  }
};
} // namespace bingo
