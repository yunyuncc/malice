#pragma once
#include <functional>
#include <map>
#include <memory>
#include <sys/epoll.h>
namespace malice::event {
class event {
public:
  // typedef std::function<void(const std::shared_ptr<event>&)> ev_handler_t;
  using ev_handler_t = std::function<void(event *e)>;
  event(int fd, int flag);
  void set_handler(int flag, ev_handler_t func);
  void fire();
  int get_flag() { return ev.events; }

private:
  int fd;
  struct epoll_event ev;
  std::map<int, ev_handler_t> handlers;
};

std::string ev_str(int flag);
} // namespace malice::event
