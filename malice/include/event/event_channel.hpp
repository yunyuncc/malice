#pragma once
//#include "base/buffer.hpp"
#include "base/tool.hpp"
#include "event/channel.hpp"
#include "event/event_loop.hpp"
#include <memory>
#include <sys/eventfd.h>
namespace malice::event {
class event_channel {
public:
  using on_event_t = std::function<void(uint64_t)>;
  event_channel(event_loop *ev_loop, on_event_t ev_handler = nullptr);
  ~event_channel();
  void notify(uint64_t val = 1);

private:
  void handle_read(::malice::base::buffer &buf);
  int fd;
  on_event_t on_event;
  std::unique_ptr<channel> chan;
};

} // namespace malice::event
