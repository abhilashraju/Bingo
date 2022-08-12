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
      buffer buf{arry.data(), arry.size()};
      int bytes = self().on_read_handler(buf);
      handler(buf);
      return true;
    });
  }
  template <typename Handler> auto sync_read(Handler handler) {
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;
    std::array<char, 1024> arry{0};
    buffer buf{arry.data(), arry.size()};
    GenericReactor &reactor = GenericReactor::get_reactor();
    std::exception_ptr eptr;
    reactor.add_handler(self().get_fd(), [&]() {
      int bytes = self().on_read_handler(buf);
      try {
        if (bytes <= 0) {
          throw std::runtime_error(std::string("socket error: ") +
                                   strerror(errno));
        }
      } catch (const std::exception &e) {
        eptr = std::current_exception(); // capture
      }
      ready = true;
      cv.notify_one();
      return bytes > 0;
    });
    std::unique_lock lk(m);
    cv.wait(lk, [&] { return ready; });
    if (eptr) {
      std::rethrow_exception(eptr);
    }
    return handler(buf);
  }
};

struct async_sock : async_stream<async_sock> {
  sock_stream sock;

  int get_fd() { return sock.fd_; }

  int on_read_handler(buffer buff) { return read(sock, buff); }
};
struct async_io : async_stream<async_io> {
  int get_fd() { return fileno(stdin); }
  int on_read_handler(buffer buff) {
    if(fgets(buff.data(), buff.length(), stdin)!=nullptr){
        return buff.length();
    }
    return 0;
        
  }
};
} // namespace bingo
