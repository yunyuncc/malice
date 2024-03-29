#pragma once
#include "base/tool.hpp"
#include <functional>
#include <map>
#include <memory>
#include <sys/epoll.h>
namespace malice::event {
class event {
public:
  using ev_handler_t = std::function<void(event *e)>;
  event(int ev_fd, int flag);
  void set_handler(int flag, ev_handler_t func);
  void fire();
  int get_flag() { return ev.events; }
  void set_flag(int flag) { ev.events = flag; }
  struct epoll_event *native_handle() {
    return &ev;
  }
  int get_fd() const { return fd; }

private:
  void check_repeat_event(int flag);
  const int fd;
  struct epoll_event ev;
  std::map<int, ev_handler_t> handlers;
};

std::string ev_str(int flag);

inline event *to_event(struct epoll_event *e) { return (event *)e->data.ptr; }
CREATE_NEW_EXCEPTION(event_mult_handler);
} // namespace malice::event
