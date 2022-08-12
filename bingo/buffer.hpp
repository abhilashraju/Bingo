#pragma once
#include <vector>
namespace bingo {
struct buffer {
  char *buff;
  size_t len;
  buffer(char *p, size_t len) : buff(p), len(len) {}
  template <size_t size> buffer(std::array<char, size> &arry) {
    buff = arry.data();
    len = arry.size();
  }
  buffer(std::vector<char> &v) {
    buff = v.data();
    len = v.capacity();
  }
  buffer(std::string &v) {
    buff = v.data();
    len = v.length();
  }
  char *data() { return buff; }
  size_t length() { return len; }
};
} // namespace bingo
