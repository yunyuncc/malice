#pragma once
#include "event/event.hpp"
#include "event/event_loop.hpp"
#include <functional>
#include <memory>
#include <string_view>
namespace malice::event {
class channel {
public:
  using on_read = std::function<void(std::string_view)>;
  using on_close = std::function<void()>;
  channel(int fd, event_loop *loop);
  ~channel();
  void enable_read(bool enable);
  void enable_write(bool enable);
  void write(std::string &buf);

private:
  //处理读事件，将数据读入缓冲区
  void handle_read(event *e);
  //重新注册event到event_loop中去
  void update_event() { ev_loop->mod_event(ev.get()); }
  static const int read_event;
  static const int write_event;
  static const int error_event;
  static const int close_event;
  static const int none_event;
  std::unique_ptr<event> ev;
  event_loop *ev_loop;
};

} // namespace malice::event
