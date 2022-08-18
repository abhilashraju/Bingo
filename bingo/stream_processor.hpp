#pragma once
#include "bingo.hpp"
#include "buffer.hpp"
#include "unifex/inplace_stop_token.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/sequence.hpp"
#include "unifex/then.hpp"
#include <string>
namespace bingo {

template <typename Stream, typename Handler> struct stream_processor {

  Stream &stream;
  unifex::inplace_stop_source stop_src;
  Handler handler;

  std::string str;
  string_buffer buf{str};
   stream_processor(Stream &s, Handler h)
      : stream(s), handler(std::move(h)) {}
  void request_stop() { stop_src.request_stop(); }
  auto handle_error(std::exception &e) {
    buf.consume_all();
    auto len = strlen(e.what());
    std::copy_n(e.what(), len, buf.prepare(len));
    buf.commit(len);
    stop_src.request_stop();
    return buf.read_view();
  }
  friend auto operator|(auto sender, stream_processor &processor) {

    auto wait_for_remote = [&]() {
      try {
        return processor.stream.sync_read(
            processor.buf, [&]() { return processor.buf.read_view(); });
      } catch (std::exception &e) {
        return processor.handle_error(e);
      }
    };
    auto readSender = sender | unifex::then(wait_for_remote);
    auto processingSender = processor.handler(readSender);
    return processingSender |
           unifex::then([&]() { processor.buf.consume_all(); }) |
           unifex::repeat_effect_until([&]() {
             return processor.stop_src.get_token().stop_requested();
           });
  }
};
} // namespace bingo
