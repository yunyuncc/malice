#pragma once
#include "base/tool.hpp"
#include "event/event.hpp"
#include <atomic>
#include <cassert>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <sys/epoll.h>
#include <thread>
#include <vector>

namespace malice::event {
//互相依赖
class event_channel;

class event_loop {
public:
  using timeout_handler_t = std::function<void()>;
  using work_t = std::function<void()>;
  event_loop(int timeout_ms);
  ~event_loop();
  void add_event(event *e);
  void del_event(event *e);
  void mod_event(event *e);
  void set_timeout_handler(timeout_handler_t func) {
    assert_in_loop_thread();
    timeout_handler = func;
  }
  void wait();
  bool in_loop_thread() {
    auto cur_id = std::this_thread::get_id();
    return thread_id == cur_id;
  }
  void assert_in_loop_thread() { assert(in_loop_thread()); }
  void loop();

  //可以在事件循环线程外调用
  void run_in_loop(work_t work);
  void wakeup();
  void stop();

private:
  void setup_wakeup_channel();
  void run_work_in_queue();
  const int fd;
  const int timeout_ms;
  std::atomic<bool> should_stop;
  const std::thread::id thread_id;

  timeout_handler_t timeout_handler;
  std::vector<work_t> work_queue;
  std::mutex m;
  std::unique_ptr<event_channel> wakeup_channel;
};
CREATE_NEW_EXCEPTION(add_event_fail);
CREATE_NEW_EXCEPTION(mod_event_fail);

} // namespace malice::event
