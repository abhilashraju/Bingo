#pragma once
#include "http_header.hpp"
namespace bingo {

template <bool isRequest, typename Body, typename Fields>
struct message : public header<isRequest, Fields> {
  
  Body& body(){return body_;}
  Body body_;
};
template <typename Body, typename Fields>
struct message<false,Body,Fields> : public header<false, Fields> {
  
  message(const Body& body, bingo::status st,unsigned version)
  :header<false, Fields>(st,version),body_(body)
  {

  }
  message(bingo::status st,unsigned version)
  :header<false, Fields>(st,version)
  {

  }
  Body& body(){return body_;}
  Body body_;
};

template <typename Body = std::string, typename Fields = fields>
using response = message<false, Body, Fields>;
template <typename Body = std::string, typename Fields = fields>
using request = message<true, Body, Fields>;

#include "http_message.ipp"
} // namespace bingo