#pragma once
#include "event/channel.hpp"
#include "event/event_loop.hpp"
#include <chrono>
#include <memory>
#include <string.h>
#include <sys/timerfd.h>
#include <utility>
namespace malice::event {
template <typename Rep, typename Period> class timer {
public:
  static constexpr int64_t nsec_per_sec = 1 * 1000 * 1000 * 1000;
  using on_time_t = std::function<void(const std::string &name)>;

  timer(event_loop *ev_loop, on_time_t func,
        const std::chrono::duration<Rep, Period> &first_expire,
        const std::chrono::duration<Rep, Period> &period =
            std::chrono::seconds(0),
        const std::string &timer_name = "default_timer_name")
      : loop(ev_loop), on_time(func), name(timer_name) {
    fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(fd != -1);
    struct itimerspec t;
    auto first_nsec =
        std::chrono::duration_cast<std::chrono::nanoseconds>(first_expire)
            .count();
    t.it_value.tv_sec = first_nsec / nsec_per_sec;
    t.it_value.tv_nsec = first_nsec % nsec_per_sec;
    auto period_nsec =
        std::chrono::duration_cast<std::chrono::nanoseconds>(period).count();
    t.it_interval.tv_sec = period_nsec / nsec_per_sec;
    t.it_interval.tv_nsec = period_nsec % nsec_per_sec;

    assert(0 == timerfd_settime(fd, 0, &t, nullptr));

    chan = std::make_unique<channel>(fd, loop);
    chan->set_read_handler(
        [this](::malice::base::buffer &buf) { handle_read(buf); });
    chan->enable_read(true);
  }
  std::string get_name() const { return name; }
  ~timer() { ::close(fd); }

private:
  void handle_read(::malice::base::buffer &buf) {
    assert(buf.readable_size() != 0);
    assert(buf.readable_size() % sizeof(int64_t) == 0);
    buf.take_all();
    if (on_time)
      on_time(name);
  }
  event_loop *loop;
  on_time_t on_time;
  std::string name;
  int fd;
  std::unique_ptr<channel> chan;
};
} // namespace malice::event
