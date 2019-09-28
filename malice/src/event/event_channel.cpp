#include "event/event_channel.hpp"
#include "base/log.hpp"
using malice::base::errno_str;
using spdlog::error;
namespace malice::event {
event_channel::event_channel(event_loop *ev_loop, on_event_t ev_handler)
    : on_event(ev_handler) {
  fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  assert(fd != -1);
  chan = std::make_unique<channel>(fd, ev_loop, sizeof(uint64_t));
  chan->set_read_handler(
      [this](::malice::base::buffer &buf) { handle_read(buf); });
  chan->enable_read(true);
}
event_channel::~event_channel() { close(fd); }
void event_channel::notify(uint64_t val) {
  int evfd = chan->get_fd();
  ssize_t n = ::write(evfd, &val, sizeof(val));
  if (n != sizeof(val)) {
    error("notify event fail, evfd={} err:{}", evfd, errno_str());
  }
}
void event_channel::handle_read(::malice::base::buffer &buf) {
  uint64_t val = buf.take_as_uint64();
  if (on_event) {
    on_event(val);
  }
}

} // namespace malice::event
