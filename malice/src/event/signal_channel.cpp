#include "event/signal_channel.hpp"
#include "base/log.hpp"
#include <atomic>
#include <string.h>
#include <unistd.h>
using spdlog::debug;
using spdlog::warn;
static std::atomic<uint8_t> signal_channel_num = 0;
namespace malice::event {

signal_channel::signal_channel(event_loop *loop) {
  if (signal_channel_num > 0) {
    throw mult_signal_channel("you can only create one signal_channel");
  }
  assert(sigfillset(&sigset) == 0);
  assert(sigdelset(&sigset, SIGKILL) == 0);
  assert(sigdelset(&sigset, SIGSTOP) == 0);
  assert(sigprocmask(SIG_BLOCK, &sigset, nullptr) == 0);
  fd = signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);
  assert(fd != -1);
  chan = std::make_unique<channel>(fd, loop);
  chan->set_read_handler(
      [this](::malice::base::buffer &buf) { handle_read(buf); });
  chan->enable_read(true);
  signal_channel_num++;
}
void signal_channel::set_signal_handler(int sig, on_signal_t func) {
  if (sigismember(&sigset, sig) != 1) {
    throw set_signal_handler_bad_param(std::string("bad sig:") +
                                       std::to_string(sig));
  }
  handlers[sig] = func;
}
void signal_channel::handle_read(::malice::base::buffer &buf) {
  struct signalfd_siginfo siginfo {};
  while (buf.readable_size() >= sizeof(siginfo)) {
    memcpy(&siginfo, buf.begin_read(), sizeof(siginfo));
    buf.has_take(sizeof(siginfo));
    int got = siginfo.ssi_signo;
    if (handlers.count(got) == 1) {
      if (handlers[got]) {
        handlers[got](siginfo);
      } else {
        debug("signal_channel ignore a signal {}", got);
      }
    } else {
      warn("signal_channel got a signal has not a handler {}", got);
    }
  }
  assert(buf.readable_size() == 0);
}
signal_channel::~signal_channel() {
  close(fd);
  assert(sigprocmask(SIG_UNBLOCK, &sigset, nullptr) == 0);
  signal_channel_num--;
}

} // namespace malice::event
