#include "event/event_loop.hpp"
#include <cassert>
#include <iostream>
#include <unistd.h>
using std::cout;
using std::endl;
namespace malice::event {
using malice::base::errno_str;

static const size_t MAX_ACTIVE_EVENTS = 256;
event_loop::event_loop(int t_ms)
    : fd(epoll_create1(EPOLL_CLOEXEC)), timeout_ms(t_ms) {
  assert(fd != -1);
}
event_loop::~event_loop() { close(fd); }
void event_loop::add_event(event *e) {
  assert(e != nullptr);
  int ret = epoll_ctl(fd, EPOLL_CTL_ADD, e->get_fd(), e->native_handle());
  if (ret == -1) {
    throw add_event_fail(errno_str());
  }
}

void event_loop::mod_event(event *e) {
  assert(e != nullptr);
  int ret = epoll_ctl(fd, EPOLL_CTL_MOD, e->get_fd(), e->native_handle());
  if (ret == -1) {
    throw mod_event_fail(errno_str());
  }
}

void event_loop::del_event(event *e) {
  epoll_ctl(fd, EPOLL_CTL_DEL, e->get_fd(), e->native_handle());
}

void event_loop::wait() {
  struct epoll_event evs[MAX_ACTIVE_EVENTS];

  int ret = epoll_wait(fd, evs, MAX_ACTIVE_EVENTS, timeout_ms);
  if (ret == -1) {
    // TODO log  error
    cout << "epoll wait error:" << errno_str() << endl;
  } else if (ret == 0) {
    if (timeout_handler)
      timeout_handler();
  } else {
    for (int i = 0; i < ret; i++) {
      struct epoll_event *ev_h = &(evs[i]);
      event *ev = to_event(ev_h);
      try {
        ev->fire();
      } catch (const std::exception &e) {
        cout << "handle event except:" << e.what() << endl;
      } catch (...) {
        cout << "handle event unknown except" << endl;
      }
    }
  }
}

} // namespace malice::event
