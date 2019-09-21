#include "event/channel.hpp"
#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using malice::base::buffer;
using malice::base::errno_str;
using std::cout;
using std::endl;
namespace malice::event {
channel::channel(int fd, event_loop *loop)
    : ev(std::make_unique<event>(fd, none_event)), ev_loop(loop) {
  ev->set_handler(read_event, [this](event *e) { handle_read(e); });
  ev->set_handler(write_event, [this](event *e) { handle_write(e); });
  ev->set_handler(error_event, [this](event *e) { handle_error(e); });
  ev->set_handler(close_event, [this](event *e) { handle_close(e); });
  on_error = [this](int e, const std::string &err_msg) {
    default_on_error(e, err_msg);
  };
  on_close = [this](channel *chan) { default_on_close(chan); };
  on_read = [this](::malice::base::buffer &buf) { default_on_read(buf); };
  ev_loop->add_event(ev.get());
}
channel::~channel() {
  ev_loop->del_event(ev.get());
  assert(read_buf.readable_size() == 0);
  if (write_buf.readable_size() == 0) {
    cout << "[warning] some data not send when channel dead" << endl;
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
//可能是EPOLLIN|EPOLLPRI,可读事件，read 返回0 EOF 调用的handle_close
// TODO 什么情况下read 不返回0 ,而是 接收到close event EPOLLHUP | EPOLLRDHUP ??
void channel::handle_close(event *e) {
  assert(e->native_handle() == ev->native_handle());
  assert(on_close);
  on_close(this);
}
void channel::handle_read(event *e) {
  assert(e->native_handle() == ev->native_handle());
  int flag = e->get_flag();
  if (flag & close_event) {
    handle_close(e);
    return;
  }
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
    handle_close(e);
  } else {
    int e = errno;
    if (e == EAGAIN || e == EWOULDBLOCK) {
      return;
    } else {
      assert(on_error);
      on_error(e, "channel::handle_read");
    }
  }
}
void channel::handle_write(event *e) {
  assert(e->native_handle() == ev->native_handle());
  int flag = e->get_flag();
  assert(flag & write_event);

  if (write_buf.readable_size() <= 0) {
    enable_write(false);
    if (on_write_finish) {
      on_write_finish(this);
    }
    return;
  }
  int fd = e->get_fd();
  ssize_t bytes = ::send(fd, write_buf.begin_read(), write_buf.readable_size(),
                         MSG_NOSIGNAL | MSG_DONTWAIT);
  if (bytes > 0) {
    write_buf.has_take(bytes);
    return;
  } else if (bytes == 0) {
    return;
  } else {
    int e = errno;
    if (e == EAGAIN || e == EWOULDBLOCK) {
      return;
    }
    assert(on_error);
    on_error(e, "channel::handle_write");
  }
}

void channel::handle_error(event *e) {
  assert(e->native_handle() == ev->native_handle());
  assert(on_error);
  std::string err_msg = "channel::handle_error[";
  int flag = e->get_flag();
  err_msg += ev_str(flag);
  err_msg += "]";
  on_error(0, err_msg);
}

void channel::default_on_read(::malice::base::buffer &buf) {
  assert(&buf == &read_buf);
  buf.take_all();
}
void channel::default_on_close(channel *c) {
  assert(c == this);
  close(c->get_fd());
}
void channel::default_on_error(int e, const std::string &err_msg) {
  std::string msg = err_msg + "  errno:" + errno_str(e);
  throw default_on_error_called(msg);
}

const int channel::read_event = EPOLLIN | EPOLLPRI;
const int channel::write_event = EPOLLOUT;
const int channel::error_event = EPOLLERR;
const int channel::close_event = EPOLLHUP | EPOLLRDHUP;
const int channel::none_event = 0;

} // namespace malice::event
