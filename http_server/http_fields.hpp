#pragma once
#include "http_field.hpp"
#include <map>
namespace bingo {
struct fields {
  std::map<std::string, std::string> fields_;
  void set(const std::string &name, std::string_view value) {
    fields_[name]= std::string{value.data(),value.length()};
  }
  void set(const std::string_view &name, std::string_view value) {
    set(std::string{name.data(),name.length()}, value);
  }
  void set(field name, std::string_view  value)
  {
    set(to_string(name),value);
  }
  const std::string_view get(const std::string &name) const {
    auto iter = fields_.find(name);
    if (iter != std::end(fields_)) {
      return std::string_view{iter->second.data(), iter->second.length()};
    }
    return std::string_view(nullptr, 0);
  }
  auto cbegin() const{
    return std::cbegin(fields_);
  }
  auto cend() const{
    return std::cend(fields_);
  }
   auto begin() {
    return std::begin(fields_);
  }
  auto end() {
    return std::end(fields_);
  }
};
} // namespace bingo
