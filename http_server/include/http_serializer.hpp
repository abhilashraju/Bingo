#pragma once
#include "http_message.hpp"
#include <strstream>
namespace bingo {
template <typename Body, typename Fields>
std::string serialize(response<Body, Fields> &res) {
  std::strstream stream;
  stream <<"HTTP/"<<res.version()/10<<"."<<res.version()%10<<" " << res.result_int() << " " << res.reason() << "\r\n";
  for (const auto &p : res) {
    stream << p.first << ":" << p.second << "\r\n";
  }
  stream << "\r\n";
  stream << res.body();
  return std::move(stream.str());
}
} // namespace bingo
