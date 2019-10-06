#include "event/event_loop_thread.hpp"
#include "base/log.hpp"
using spdlog::error;
namespace malice::event {

event_loop_thread::event_loop_thread(const std::string &name, work_t begin_work,
                                     work_t end_work)
    : thread_name(name), begin_func(begin_work), end_func(end_work) {}
event_loop_thread::~event_loop_thread() {}

void event_loop_thread::start() {
  th = std::make_unique<std::thread>([this] { thread_func(); });
  std::unique_lock<std::mutex> lock(mtx);
  started_cv.wait(lock, [this] { return (loop ? true : false); });
}

void event_loop_thread::stop() {
  if (!th)
    return;
  loop->stop();
  th->join();
  th.reset();
}

void event_loop_thread::run(work_t work) {
  assert(work != nullptr);
  assert(loop);
  loop->run_in_loop(work);
}

void event_loop_thread::thread_func() {
  if (begin_func)
    begin_func();
  {
    std::unique_lock<std::mutex> lock(mtx);
    loop = std::make_unique<event_loop>(-1);
  }
  started_cv.notify_one();
  loop->loop();
  if (end_func)
    end_func();
  loop.reset();
}

} // namespace malice::event
