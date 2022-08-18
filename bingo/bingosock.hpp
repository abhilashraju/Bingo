#pragma once
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>
#include <buffer.hpp>
namespace bingo {
struct sock_address {
  struct sockaddr_in address {
    0
  };
  sock_address() {}
  sock_address(std::string addr, int port) {
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (int r = inet_pton(AF_INET, addr.data(), &address.sin_addr) <= 0) {
      throw std::runtime_error(std::string("address error: ") + strerror(r));
    }
  }
};
struct sock_stream {
  int fd_{-1};
  sock_address address;
  bool listening{false};
  sock_stream() {
    if ((fd_ = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      throw std::runtime_error(std::string("sock creation error: ") +
                               strerror(errno));
    }
  }
  sock_stream(int fd, sock_address addr, bool l)
      : fd_(fd), address(addr), listening(l) {}
  sock_stream(const sock_stream &other) = delete;
  sock_stream &operator=(const sock_stream &other) = delete;
  sock_stream(sock_stream &&other)
      : fd_(other.fd_), address(std::move(other.address)),
        listening(other.listening) {
    other.fd_ = -1;
    other.listening = false;
  }
  sock_stream &bind(sock_address addr) {
    address = addr;
    if (int r = ::bind(fd_, (struct sockaddr *)&address.address,
                       sizeof(addr.address)) < 0) {
      throw std::runtime_error(std::string("bind error: ") + strerror(r));
    }
    return *this;
  }
  friend void swap(sock_stream &t1, sock_stream &t2) {
    std::swap(t1.fd_, t2.fd_);
    std::swap(t1.listening, t2.listening);
    std::swap(t1.address, t2.address);
  }
  sock_stream &operator=(sock_stream &&other) {
    sock_stream temp(std::move(other));
    swap(*this, temp);
    return *this;
  }
  ~sock_stream() {
    if (listening) {
      shutdown(*this);
      listening = false;
    }
    if (fd_ > 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }
  friend void shutdown(sock_stream &stream) { shutdown(stream.fd_, SHUT_RDWR); }
  friend void listen(sock_stream &stream) {
    if (::listen(stream.fd_, 3) < 0) {
      throw std::runtime_error(std::string("listen error: ") + strerror(errno));
    }
  }
  friend void listen(sock_stream &stream, const sock_address &addr) {
    stream.bind(addr);
    listen(stream);
  }
  friend void set_blocked(sock_stream &stream) {
    if (int r = fcntl(stream.fd_, F_SETFL,
                      fcntl(stream.fd_, F_GETFL) | F_LOCK) < 0) {
      throw std::runtime_error(std::string("block failed error: ") +
                               strerror(r));
    }
  }
  friend sock_stream accept(const sock_stream &stream) {

    sock_address address;
    int addrlen{sizeof(address.address)};
    int fd{-1};
    if ((fd = ::accept(stream.fd_, (struct sockaddr *)&address.address,
                       (socklen_t *)&addrlen)) < 0) {
      throw std::runtime_error(std::string("accept error: ") + strerror(errno));
    }
    sock_stream new_socket{fd, address, false};
    set_blocked(new_socket);
    return new_socket;
  }

  template <typename Buffer>
  friend int read(const sock_stream &stream, Buffer& buff) {
    constexpr int MAXSIZE=1024;
    auto read=0;
    while(true){
       int r = ::read(stream.fd_, buff.prepare(MAXSIZE), MAXSIZE);
       if (r < 0) {
          if(errno==EINTR) continue;
          throw std::runtime_error(strerror(r));
       }
       read+=r;
       buff.commit(r);
       if(r<MAXSIZE){
        break;
       }
    }
    return read;
  }
  template <typename Buffer>
  friend int send(const sock_stream &stream, Buffer buff) {
    int r = ::send(stream.fd_, buff.data(), buff.read_length(), MSG_NOSIGNAL);
    if (r < 0) {
      throw std::runtime_error(strerror(r));
    }
    return r;
  }
 
  friend sock_stream connect(sock_stream &stream,
                             const sock_address &serv_addr) {
    sock_stream newstream;
    if ((newstream.fd_ =
             ::connect(stream.fd_, (struct sockaddr *)&serv_addr.address,
                       sizeof(serv_addr.address))) < 0) {
      throw std::runtime_error(std::string("connection error: ") +
                               strerror(errno));
    }
    set_blocked(newstream);
    return newstream;
  }
};

} // namespace bingo
