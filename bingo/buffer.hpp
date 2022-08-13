#pragma once
#include <vector>
namespace bingo {
struct NullResizer {
  char *resize(char *data, size_t current, size_t req) {
    throw std::runtime_error("Buffer Cannot be resized");
  }
};
struct VectorResizer {
  std::vector<char> *vec{nullptr};
  VectorResizer(){}
  void set_vector(std::vector<char>& v){vec=&v;}
  char *resize(char *data, size_t current, size_t req) {
    vec->resize(req);
    return vec->data();
  }
};
struct StringResizer {
  std::string *str{nullptr};
  StringResizer() {}
  void set_string(std::string& s){str=&s;}
  char *resize(char *data, size_t current, size_t req) {
    str->resize(req);
    return str->data();
  }
};

template <typename Resizer> struct buffer_base {
  char *base{nullptr};
  const char *read_buff{nullptr};
  size_t read_len{0};
  char *write_buff{nullptr};
  size_t write_len{0};
  size_t capacity{0};
  Resizer resizer;
  void intialise(char *p, size_t len) {
    base = p;
    read_buff = p;
    read_len = len;
    write_buff = p;
    write_len = 0;
    capacity = len;
  }
  buffer_base(char *p, size_t len, NullResizer res = NullResizer()) : resizer(res) {
    intialise(p, len);
    write_buff=base+len;
  }
  template <size_t size>
  buffer_base(std::array<char, size> &arry, NullResizer res = NullResizer())
      : resizer(res) {
    intialise(arry.data(), arry.length());
  }
  buffer_base(std::vector<char> &v, VectorResizer res = VectorResizer())
      : resizer(res) {
     resizer.set_vector(v);
    intialise(v.data(), v.capacity());
    capacity = v.capacity();
  }
  buffer_base(std::string &v, StringResizer res = StringResizer()) : resizer(res) {
    resizer.set_string(v);
    intialise(v.data(), v.size());
  }

  const char *data() { return read_buff; }
  const char *read_begin() { return read_buff; }
  const char *read_end() { return read_buff + read_len; }
  size_t read_length() { return read_len; }
  void consume(size_t n) {
    if(read_buff +n > write_buff || n > read_len){
      throw std::runtime_error("read buffer pointer overflows write buffer regions");
    }
    read_buff += n;
    read_len -= n;
  }
  void resize(size_t newsize) {
    auto oldbase = base;
    base = resizer.resize(base, capacity, newsize);
    read_buff = base + (read_buff - oldbase);
    write_buff = base + (write_buff - oldbase);
    std::fill(write_buff,write_buff+write_len,0);
    capacity=newsize;
  }
  char *prepare(size_t n) {
    auto overshoot = (write_buff + n) - (base + capacity);
    if (overshoot > 0) {
      resize(capacity + overshoot);
    }
    write_len = n;
    return write_buff;
  }
  void commit(size_t used) {
    auto usedbuff = write_buff + used;
    auto end = base + capacity;
    write_buff = (usedbuff >= end) ? end : usedbuff;
    read_len += used;
    write_len-= used;
  }
  char *write_begin() { return write_buff; }
  char *writer_end() { return write_buff + write_len; }
  size_t write_length() { return write_len; }

  size_t length() { return capacity; }
};

using buffer = buffer_base<NullResizer>;
using string_buffer=buffer_base<StringResizer>;
using vector_buffer=buffer_base<VectorResizer>;
} // namespace bingo
