#pragma once
#include <exception>
#include <string.h>
namespace bingo {
struct socket_exception : std::runtime_error {
  socket_exception(const std::string &message) : runtime_error(message) {}
};
struct application_error:std::runtime_error{
  application_error(const std::string &message) : runtime_error(message) {}
};
} // namespace bingo
