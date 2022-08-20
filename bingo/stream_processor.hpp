#pragma once
#include "bingo.hpp"
#include "buffer.hpp"
#include "unifex/inplace_stop_token.hpp"
#include "unifex/repeat_effect_until.hpp"
#include "unifex/sequence.hpp"
#include "unifex/then.hpp"
#include <string>
namespace bingo {

template <typename Stream, typename Handler, typename Error_Handler>
struct stream_processor {

  Stream &stream;
  unifex::inplace_stop_source stop_src;
  Handler handler;
  Error_Handler error_handler;

  std::string str;
  string_buffer buf{str};
  stream_processor(Stream &s, Handler h, Error_Handler e_h)
      : stream(s), handler(std::move(h)), error_handler(std::move(e_h)) {}
  void request_stop() { stop_src.request_stop(); }

  template <typename Sender>
  friend auto operator|(Sender sender, stream_processor &processor) {

    auto wait_for_remote = [&]() {
      std::exception_ptr eptr;
      try {
        return processor.stream.sync_read(
            processor.buf, [&]() { return processor.buf.read_view(); });
      } catch (...) {
        eptr = std::current_exception(); // capture
      }
      processor.error_handler(eptr);
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

auto wait_for_io(auto &io_object, auto& buf) {
  return unifex::then([&](){
    return io_object.sync_read(buf,
                     [&]() { return buf.read_view(); });
  });
}
} // namespace bingo
