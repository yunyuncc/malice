#pragma once
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
  const int fd;
  struct epoll_event ev;
  std::map<int, ev_handler_t> handlers;
};

std::string ev_str(int flag);

inline event *to_event(struct epoll_event *e) { return (event *)e->data.ptr; }
} // namespace malice::event
