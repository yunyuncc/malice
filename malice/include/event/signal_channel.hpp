#pragma once

#include "base/tool.hpp"
#include "event/channel.hpp"
#include "event/event_loop.hpp"
#include <memory>
#include <signal.h>
#include <sys/signalfd.h>
namespace malice::event {
class signal_channel {
public:
  using on_signal_t = std::function<void(const signalfd_siginfo &siginfo)>;
  signal_channel(event_loop *ev_loop);
  ~signal_channel();
  void ignore(int sig) { set_signal_handler(sig, nullptr); };
  void set_signal_handler(int sig, on_signal_t func);

private:
  void handle_read(::malice::base::buffer &buf);
  sigset_t sigset;
  int fd;
  std::unique_ptr<channel> chan;
  std::map<int, on_signal_t> handlers;
};

CREATE_NEW_EXCEPTION(mult_signal_channel);
CREATE_NEW_EXCEPTION(set_signal_handler_bad_param);

} // namespace malice::event
