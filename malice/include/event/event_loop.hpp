#pragma once
#include "base/tool.hpp"
#include "event/event.hpp"
#include <functional>
#include <stdexcept>
#include <sys/epoll.h>
namespace malice::event {

class event_loop {
public:
  using timeout_handler_t = std::function<void()>;
  event_loop(int timeout_ms);
  ~event_loop();
  // TODO assert in loop thread
  void add_event(event *e);
  void del_event(event *e);
  void mod_event(event *e);
  void set_timeout_handler(timeout_handler_t func) { timeout_handler = func; }
  void wait();

private:
  const int fd;
  const int timeout_ms;
  timeout_handler_t timeout_handler;
};

CREATE_NEW_EXCEPTION(add_event_fail);
CREATE_NEW_EXCEPTION(mod_event_fail);

} // namespace malice::event
