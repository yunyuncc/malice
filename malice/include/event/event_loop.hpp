#pragma once
#include "base/tool.hpp"
#include "event/event.hpp"
#include <cassert>
#include <functional>
#include <stdexcept>
#include <sys/epoll.h>
#include <thread>
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
  void set_timeout_handler(timeout_handler_t func) {
    assert_in_loop_thread();
    timeout_handler = func;
  }
  void wait();
  bool in_loop_thread() { return thread_id == std::this_thread::get_id(); }
  void assert_in_loop_thread() { assert(in_loop_thread()); }

private:
  const int fd;
  const int timeout_ms;
  timeout_handler_t timeout_handler;
  std::thread::id thread_id;
};
CREATE_NEW_EXCEPTION(add_event_fail);
CREATE_NEW_EXCEPTION(mod_event_fail);

} // namespace malice::event
