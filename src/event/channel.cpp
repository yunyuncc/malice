#include "event/channel.hpp"
namespace malice::event {
channel::channel(int fd, event_loop *loop)
    : ev(std::make_unique<event>(fd, none_event)), ev_loop(loop) {
  ev_loop->add_event(ev.get());
}
channel::~channel() { ev_loop->del_event(ev.get()); }
void channel::enable_read(bool enable) {
  if (enable) {
    int flag = ev->get_flag();
    flag |= read_event;
    ev->set_flag(flag);
    update_event();
  } else {
    int flag = ev->get_flag();
    flag &= ~read_event;
  }
}
void channel::enable_write(bool enable) {
  if (enable) {
    int flag = ev->get_flag();
    flag |= write_event;
    ev->set_flag(flag);
    update_event();
  } else {
    int flag = ev->get_flag();
    flag &= ~write_event;
  }
}

const int channel::read_event = EPOLLIN | EPOLLPRI;
const int channel::write_event = EPOLLOUT;
const int channel::error_event = EPOLLERR;
const int channel::close_event = EPOLLHUP | EPOLLRDHUP;
const int channel::none_event = 0;

} // namespace malice::event
