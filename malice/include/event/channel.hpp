#pragma once
#include "base/buffer.hpp"
#include "base/tool.hpp"
#include "event/event.hpp"
#include "event/event_loop.hpp"
#include <functional>
#include <memory>
namespace malice::event {
// 1.channel 只封装了 fd的基于事件的读写操作，并不负责打开和关闭fd
// 2.enable_read(true)并且挂载on_read的时候channel就会自动从接受缓存区读数据到read_buf然后回调on_read
// 3.enable_read(true)并且不挂载on_read，就会将接收缓冲区中的数据读出来丢掉(default_on_read)
// 5. a:实现on_close应该负责关闭fd,否则fd会泄露
// b:不实现on_close,default_on_close会关闭fd
// 6.在读写fd,或者epoll_wait激活的事件中发生EPOLLERR的时候on_error会被调用，如果不挂载on_error的话，默认的on_error就会丢异常出来，event_loop循环就会捕捉到该异常，并打印日志
// 8.构造channel的时候就会将fd,注册到event_loop中去，并关注该fd的事件
// 9.析构channel的时候会将fd从event_loop中移除
// 10.怎么避免channel析构的时候read_buf,write_buf中没有剩余的未消费的数据？TODO
// add a warning log 11.可以使能或关闭channel的读事件(TODO
// 如果local关闭读事件，peer又一直在写数据，会怎么样?
// 猜测会将local的socket读缓冲区和peer的写缓冲区占满，这样peer的可写事件就不会被激活，连接处于挂起状态，直到local继续读数据)
class channel {
public:
  // close type :
  //对端close的EPOLLIN(read 0),EPOLLRDHUP
  // EPOLLHUP
  //这两种事件的处理可能不一样，
  // socketpair无法复现EPOLLRDHUP,应该是只有socket close才会有？TODO
  enum class close_type { eof = 0, close_event };
  using on_read_t = std::function<void(::malice::base::buffer &buf)>;
  using on_close_t = std::function<void(channel *chan, close_type ct)>;
  using on_error_t =
      std::function<void(int, const std::string &msg, channel *chan)>;
  using on_write_finish_t = std::function<void(channel *chan)>;

  channel(int fd, event_loop *loop,
          size_t init_buf_size = ::malice::base::buffer::init_size);
  ~channel();
  bool buffer_empty() const {
    return (read_buffer_empty() && write_buffer_empty());
  }
  bool read_buffer_empty() const { return read_buf.readable_size() == 0; }
  bool write_buffer_empty() const { return write_buf.readable_size() == 0; }
  void enable_read(bool enable);
  int get_fd() const { return ev->get_fd(); }
  void write(const std::string &buf) {
    write(static_cast<const void *>(buf.data()), buf.size());
  }
  // TODO 需要线程安全
  void write(const void *data, size_t len) {
    write_buf.append(data, len);
    enable_write(true);
  }
  void set_read_handler(on_read_t func) { on_read = func; }
  // 1:close when got a EOF
  // 2:close when got a close event
  void set_close_handler(on_close_t func) { on_close = func; }
  // 1:error when read (TODO)
  // 2:error when write(write on close fd)
  // 3:error from event(TODO)
  void set_error_handler(on_error_t func) { on_error = func; }
  void set_write_finish_handler(on_write_finish_t func) {
    on_write_finish = func;
  }

private:
  void enable_write(bool enable);
  //处理close事件，如果挂载了on_close就调用on_close,如果没有就直接close掉fd
  void handle_close(event *e);
  //处理读事件，将数据读入read_buf
  void handle_read(event *e);
  //处理写事件，将write_buf中的数据写入fd
  void handle_write(event *e);
  void handle_error(event *e);
  void default_on_error(int e, const std::string &err_msg);
  void default_on_close(channel *c, close_type ct);
  void default_on_read(::malice::base::buffer &buf);
  //重新注册event到event_loop中去
  void update_event() { ev_loop->mod_event(ev.get()); }
  static const int read_event;
  static const int write_event;
  static const int error_event;
  static const int close_event;
  static const int none_event;
  std::unique_ptr<event> ev;
  event_loop *ev_loop;
  ::malice::base::buffer read_buf;
  ::malice::base::buffer write_buf;
  on_read_t on_read;
  on_close_t on_close;
  on_error_t on_error;
  on_write_finish_t on_write_finish;
};

CREATE_NEW_EXCEPTION(default_on_error_called);
} // namespace malice::event
