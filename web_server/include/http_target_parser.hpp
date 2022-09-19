#pragma once
#include <numeric>
#include <string>
#include <vector>

namespace bingo {
struct http_function {
  struct parameter {
    std::string name;
    std::string value;
    parameter(std::string n, std::string v)
        : name(std::move(n)), value(std::move(v)) {}
  };
  using parameters = std::vector<parameter>;
  std::string _name;
  parameters _params;
  const auto &name() const { return _name; }
  const auto &params() const { return _params; }
  std::string operator[](const std::string &name) const {
    if (auto iter = std::find_if(begin(_params), end(_params),
                                 [&](auto &p) { return p.name == name; });
        iter != end(_params)) {
      return iter->value;
    }
    return std::string();
  }
};
std::string to_string(std::string_view vw) {
  return std::string(vw.data(), vw.length());
}
inline std::vector<std::string> split(std::string_view sv, char delim) {
  std::vector<std::string> res;
  return std::accumulate(begin(sv), end(sv), res, [&](auto &sofar, auto c) {
    if (sofar.size() == 0) {
      sofar.emplace_back(std::string() + c);
      return sofar;
    }
    if (c == delim) {
      sofar.emplace_back();
      return sofar;
    }
    sofar.back() += c;
    return sofar;
  });
}
inline http_function parse_function(std::string_view target) {
  auto index = target.find_last_of("/");
  if (index != std::string::npos) {
    auto func = target.substr(0, index);
    auto paramstring = target.substr(index, target.length() - index);
    auto funcindex = paramstring.find_first_of("?");
    if (funcindex != std::string::npos) {
      auto params = split(
          paramstring.substr(funcindex + 1, paramstring.length() - funcindex),
          '&');
      http_function::parameters parampairs;
      for (auto &p : params) {
        auto pairs = split(p, '=');
        parampairs.emplace_back(pairs[0], pairs[1]);
      }
      return http_function{to_string(func) +
                               to_string(paramstring.substr(0, funcindex)),
                           std::move(parampairs)};
    }
    return http_function{to_string(target), http_function::parameters{}};
  }
  return http_function{};
}
} // namespace bingo