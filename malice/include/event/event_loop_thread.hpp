#pragma once
#include "event/event_channel.hpp"
#include "event/event_loop.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
namespace malice::event {
class event_loop_thread {
public:
  using work_t = std::function<void()>;
  event_loop_thread(const std::string &name = "", work_t begin_work = nullptr,
                    work_t end_work = nullptr);
  ~event_loop_thread();
  void run(work_t work);
  void start();
  void stop();

private:
  void thread_func();
  std::string thread_name;
  work_t begin_func;
  work_t end_func;
  std::unique_ptr<std::thread> th;
  std::unique_ptr<event_loop> loop;
  std::mutex mtx;
  std::condition_variable started_cv;
};

} // namespace malice::event
