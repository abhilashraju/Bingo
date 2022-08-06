#pragma once
#include "bingosock.hpp"
#include "reactor.hpp"
#include <condition_variable>
#include <iostream>
#include <stdio.h>
namespace bingo {
template <typename stream> struct async_stream {

  stream &self() { return static_cast<stream &>(*this); }
  template <typename Handler> auto on_read(Handler handler) {

    GenericReactor &reactor = GenericReactor::get_reactor();
    reactor.add_handler(self().get_fd(), [&]() {
      std::array<char, 1024> arry{0};
      Buffer buffer{arry.data(), arry.size()};
      int bytes = self().on_read_handler(buffer);
      handler(buffer);
      return true;
    });
  }
  template <typename Handler> auto sync_read(Handler handler) {
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;
    std::array<char, 1024> arry{0};
    Buffer buffer{arry.data(), arry.size()};
    GenericReactor &reactor = GenericReactor::get_reactor();
    reactor.add_handler(self().get_fd(), [&]() {
      int bytes = self().on_read_handler(buffer);
      ready = true;
      cv.notify_one();
      return true;
    });
    std::unique_lock lk(m);
    cv.wait(lk, [&] { return ready; });
    return handler(buffer);
  }
};

struct async_sock : async_stream<async_sock> {
  sock_stream sock;

  int get_fd() { return sock.fd_; }

  int on_read_handler(Buffer buff) { return read(sock, buff); }
};
struct async_io : async_stream<async_io> {
  int get_fd() { return fileno(stdin); }
  int on_read_handler(Buffer buff) {
    fgets(buff.buffer(), buff.length(), stdin);
    return buff.length();
  }
};
} // namespace bingo
