#include "event/event_loop.hpp"
#include "base/log.hpp"
#include "event/event_channel.hpp"
#include <cassert>
#include <unistd.h>

namespace malice::event {
using malice::base::errno_str;
using namespace spdlog;

__thread event_loop *loop_of_this_thread =
    nullptr; //保证一个线程里面只有一个event_loop

static const size_t MAX_ACTIVE_EVENTS = 256;
event_loop::event_loop(int t_ms)
    : fd(epoll_create1(EPOLL_CLOEXEC)), timeout_ms(t_ms), should_stop(false) {
  assert(fd != -1);
  assert(loop_of_this_thread == nullptr);
  thread_id = std::this_thread::get_id();

  loop_of_this_thread = this;

  //这个函数会将this指针暴漏给event_channel
  // event_channel会调用event_loop的成员函数
  //就会出现调用event_loop成员函数(add_event,
  //mod_event)的时候event_loop还没完全构造完成 这样的模式应该是有安全隐患, FIXME
  setup_wakeup_channel();
}

void event_loop::setup_wakeup_channel() {
  std::unique_ptr<event_channel> chan = std::make_unique<event_channel>(this);
  wakeup_channel.swap(chan);
}
event_loop::~event_loop() {
  ::close(fd);
  loop_of_this_thread = nullptr;
}
void event_loop::add_event(event *e) {
  assert(e != nullptr);
  assert_in_loop_thread();
  int ret = epoll_ctl(fd, EPOLL_CTL_ADD, e->get_fd(), e->native_handle());
  if (ret == -1) {
    throw add_event_fail(errno_str());
  }
}

void event_loop::mod_event(event *e) {
  assert(e != nullptr);
  assert_in_loop_thread();
  int ret = epoll_ctl(fd, EPOLL_CTL_MOD, e->get_fd(), e->native_handle());
  if (ret == -1) {
    throw mod_event_fail(errno_str() + " event_loop::mod_event");
  }
}

void event_loop::del_event(event *e) {
  assert_in_loop_thread();
  epoll_ctl(fd, EPOLL_CTL_DEL, e->get_fd(), e->native_handle());
}

void event_loop::wait() {
  assert_in_loop_thread();
  // TODO 应该使MAX_ACTIVE_EVENTS的大小是自适应的
  struct epoll_event evs[MAX_ACTIVE_EVENTS];

  int ret = epoll_wait(fd, evs, MAX_ACTIVE_EVENTS, timeout_ms);
  if (ret == -1) {
    error("epoll wait error:{}", errno_str());
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
        error("process event got exception[{}]", e.what());
      } catch (...) {
        error("process event got unknown exception");
      }
    }
  }
}
void event_loop::stop() {
  should_stop = true;
  wakeup();
}
void event_loop::run_work_in_queue() {
  std::vector<work_t> cur_works;
  {
    std::lock_guard<std::mutex> lock(m);
    cur_works = work_queue;
    work_queue.clear();
  }
  for (work_t work : cur_works) {
    assert(work);
    work();
  }
}
void event_loop::loop() {
  while (!should_stop) {
    wait();
    run_work_in_queue();
  }
}
void event_loop::run_in_loop(work_t work) {
  assert(work != nullptr);
  if (in_loop_thread()) {
    work();
    return;
  }
  {
    std::lock_guard<std::mutex> lock(m);
    work_queue.emplace_back(work);
  }
  wakeup();
}
void event_loop::wakeup() {
  assert(wakeup_channel);
  wakeup_channel->notify();
}

} // namespace malice::event
