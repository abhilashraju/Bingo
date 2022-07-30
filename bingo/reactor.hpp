#pragma once
#include "bingosock.hpp"
#include <functional>
#include <vector>
namespace bingo {
template <typename NewConnHandler, typename ReadHandler> struct Reactor {
  NewConnHandler new_conn_handler;
  ReadHandler read_handler;
  static constexpr int MAXCLIENTS = 100;
  Reactor(NewConnHandler conn_handler, ReadHandler read_handler)
      : new_conn_handler(std::move(conn_handler)),
        read_handler(std::move(read_handler)) {}
  void run(sock_stream &listener) {
    std::array<int, MAXCLIENTS> clientFd{-1};
    std::fill_n(begin(clientFd), MAXCLIENTS, -1);
    int maxSockFd = listener.fd_;
    int maxFdIndex = -1;
    fd_set allset, rset;
    FD_ZERO(&allset);
    FD_SET(listener.fd_, &allset);
    while (true) {
      rset = allset;
      int nready = select(maxSockFd + 1, &rset, nullptr, nullptr, nullptr);
      if (FD_ISSET(listener.fd_, &rset)) {
        auto newsock = accept(listener);
        auto iter = std::find(begin(clientFd), end(clientFd), -1);
        if (iter != std::end(clientFd)) {
          *iter = newsock.fd_;
          if (maxSockFd < newsock.fd_) {
            maxSockFd = newsock.fd_;
          }
          FD_SET(newsock.fd_, &allset);
          auto distance = std::distance(begin(clientFd), iter);
          if (distance >= maxFdIndex) {
            maxFdIndex = distance+1;
          }
          new_conn_handler(std::move(newsock));
        }
        if (--nready == 0)
          continue; // no more descriptor to handle
      }
      auto replace=[&](auto fd) {
            if (fd >= 0 && FD_ISSET(fd, &rset) && nready > 0) {
              if (!read_handler(fd)) {
                FD_CLR(fd, &allset);
                return true;
              }
              --nready;
            }
            return false;
          };
      std::replace_if(begin(clientFd), begin(clientFd)+maxFdIndex,replace,-1);
    }
  }
};
} // namespace bingo
