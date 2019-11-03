#pragma once

#include "base/tool.hpp"
#include "event/channel.hpp"
#include "event/event_loop.hpp"
#include <memory>
#include <signal.h>
#include <sys/signalfd.h>
namespace malice::event {

// signal_channel 创建一个signalfd,并注册到event_loop中
//一个进程只能有一个signal_channel
// signal_channel会拦截除SIGKILL SIGSTOP外的所有信号
//需要为每个signal注册一个handler,这样当信号发生的时候handler就会在event_loop所在的线程被回调
//如果signal_channel接受到了一个signal但是没有注册响应的handler，就会打印一条warn日志，然后将该信号忽略
//如果调用ignore忽略某个信号，那么当信号发生的时候就会直接忽略该信号，并且打印一条debug日志
class signal_channel {
public:
  using on_signal_t = std::function<void(const signalfd_siginfo &siginfo)>;
  signal_channel(std::shared_ptr<event_loop> ev_loop);
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
