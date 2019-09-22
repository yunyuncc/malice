#include "event/event.hpp"
#include <map>
namespace malice::event {
static std::map<int, std::string> flag_event_map{
    {EPOLLIN, "EPOLLIN"},         {EPOLLOUT, "EPOLLOUT"},
    {EPOLLRDHUP, "EPOLLRDHUP"},   {EPOLLPRI, "EPOLLPRI"},
    {EPOLLERR, "EPOLLERR"},       {EPOLLHUP, "EPOLLHUP"},
    {EPOLLET, "EPOLLET"},         {EPOLLONESHOT, "EPOLLONESHOT"},
    {EPOLLWAKEUP, "EPOLLWAKEUP"}, {EPOLLEXCLUSIVE, "EPOLLEXCLUSIVE"}};

event::event(int ev_fd, int flag) : fd(ev_fd) {
  ev.events = flag;
  ev.data.ptr = this;
}
//检查flag中是否有已经挂载过的event,如果有就丢异常，保证一个event不会有多个handler
void event::check_repeat_event(int flag) {
  int old_flag = 0;
  for (auto it = handlers.begin(); it != handlers.end(); ++it) {
    if (flag & it->first) {
      old_flag |= it->first;
    }
  }
  if (old_flag != 0) {
    throw event_mult_handler(ev_str(old_flag));
  }
}
void event::set_handler(int flag, ev_handler_t func) {
  check_repeat_event(flag);
  handlers[flag] = func;
}
//调用当前所发生的所有的event的handler
void event::fire() {
  for (auto [k, v] : handlers) {
    if (ev.events & k) {
      v(this);
    }
  }
}

std::string ev_str(int flag) {
  std::string res;
  for (auto [k, v] : flag_event_map) {
    if (k & flag) {
      res += v;
      res += "|";
    }
  }
  if (!res.empty()) {
    auto it = res.end();
    it--;
    if (*it == '|') {
      res = res.substr(0, res.size() - 1);
    }
  }
  return res;
}

} // namespace malice::event
