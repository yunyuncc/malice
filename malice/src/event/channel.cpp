#include "event/channel.hpp"
#include "base/log.hpp"
#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace spdlog;
using malice::base::buffer;
using malice::base::errno_str;
namespace malice::event {
channel::channel(int fd, event_loop *loop)
    : ev(std::make_unique<event>(fd, none_event)), ev_loop(loop) {
  ev->set_handler(read_event, [this](event *e) { handle_read(e); });
  ev->set_handler(write_event, [this](event *e) { handle_write(e); });
  ev->set_handler(error_event, [this](event *e) { handle_error(e); });
  ev->set_handler(close_event, [this](event *e) { handle_close(e); });
  on_error = [this](int e, const std::string &err_msg, channel *) {
    default_on_error(e, err_msg);
  };
  on_close = [this](channel *chan, close_type ct) {
    default_on_close(chan, ct);
  };
  on_read = [this](::malice::base::buffer &buf) { default_on_read(buf); };
  ev_loop->add_event(ev.get());
}
channel::~channel() {
  ev_loop->del_event(ev.get());
  assert(read_buf.readable_size() == 0);
  if (write_buf.readable_size() != 0) {
    warn("some data not send when channel dead");
  }
}
void channel::enable_read(bool enable) {
  if (enable) {
    int flag = ev->get_flag();
    flag |= read_event;
    ev->set_flag(flag);
    update_event();
  } else {
    int flag = ev->get_flag();
    flag &= ~read_event;
  }
}
void channel::enable_write(bool enable) {
  if (enable) {
    int flag = ev->get_flag();
    flag |= write_event;
    ev->set_flag(flag);
    update_event();
  } else {
    int flag = ev->get_flag();
    flag &= ~write_event;
  }
}
//发生close event的时候回调
// TODO 什么情况下可以接收到close event EPOLLHUP | EPOLLRDHUP ??
void channel::handle_close(event *e) {
  assert(e->native_handle() == ev->native_handle());
  assert(on_close);
  on_close(this, close_type::close_event);
}
void channel::handle_read(event *e) {
  assert(e->native_handle() == ev->native_handle());
  int flag = e->get_flag();
  assert(flag & read_event);
  int fd = e->get_fd();
  if (read_buf.writable_size() == 0) {
    read_buf.ensure_writable(buffer::init_size);
  }
  ssize_t bytes = read(fd, read_buf.begin_write(), read_buf.writable_size());
  if (bytes > 0) {
    read_buf.has_writen(bytes);
    assert(on_read);
    on_read(read_buf);
  } else if (bytes == 0) {
    assert(on_close);
    on_close(this, close_type::eof);
  } else {
    int e = errno;
    if (e == EAGAIN || e == EWOULDBLOCK) {
      return;
    } else {
      assert(on_error);
      on_error(e, "channel::handle_read", this);
    }
  }
}
//处理write_event,当fd关注了可写事件，并且发生了可写事件的时候回调
// 1.写数据
// 2.如果write_buf中的数据都写完了，回调on_write_finish，取消关注可写事件
void channel::handle_write(event *e) {
  assert(e->native_handle() == ev->native_handle());
  int fd = e->get_fd();
  ssize_t bytes =
      ::send(fd,
             write_buf.begin_read(), write_buf.readable_size(), MSG_NOSIGNAL /* TODO fd本身就是noblock的时候加这个flag会不会损耗性能? | MSG_DONTWAIT*/);
  if (bytes >= 0) {
    write_buf.has_take(bytes);
  } else {
    int e = errno;
    if (e == EAGAIN || e == EWOULDBLOCK) {
      return;
    }
    assert(on_error);
    on_error(e, "channel::handle_write", this);
  }
  //检查write_buf是否写完
  if (write_buf.readable_size() <= 0) {
    enable_write(false);
    if (on_write_finish) {
      on_write_finish(this);
    }
  }
}

//处理error_event当发生了event EPOLLERR的时候会回调本函数
void channel::handle_error(event *e) {
  assert(e->native_handle() == ev->native_handle());
  assert(on_error);
  std::string err_msg = "channel::handle_error[";
  int flag = e->get_flag();
  err_msg += ev_str(flag);
  err_msg += "]";
  on_error(0, err_msg, this);
}

//默认的读事件处理函数
//行为是将从fd中读到的数据全部丢弃
void channel::default_on_read(::malice::base::buffer &buf) {
  assert(&buf == &read_buf);
  buf.take_all();
}
//默认的连接关闭处理函数
//行为是将本channel的fd close掉,暂时不关心close_type
void channel::default_on_close(channel *c, close_type) {
  assert(c == this);
  close(c->get_fd());
}
//默认的错误处理函数
//行为是将fd close掉，并打印错误日志
void channel::default_on_error(int e, const std::string &err_msg) {
  std::string msg = err_msg + "  errno:" + errno_str(e);
  int ret = close(get_fd());
  error("default_on_error:{} {} close ret:{}", e, msg, ret);
  // throw default_on_error_called(msg);
}
//可读事件，读到0表明peer要么调用了close
//要么调用了shutdown发来的FIN,此时程序是被动关闭，可以直接close，也可以写点数据再close
const int channel::read_event = EPOLLIN | EPOLLPRI;
const int channel::write_event = EPOLLOUT;
// TODO 这个事件什么时候会出现？
const int channel::error_event = EPOLLERR;
// TODO 验证一下这两个事件到底什么情况下会复现
const int channel::close_event = EPOLLHUP | EPOLLRDHUP;
const int channel::none_event = 0;

} // namespace malice::event
