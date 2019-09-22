#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/channel.hpp"
#include <atomic>
#include <doctest/doctest.h>
#include <sys/socket.h>
#include <sys/types.h>
using namespace malice::event;
using namespace spdlog;
TEST_CASE("create  a channel") {
  event_loop loop(-1);
  channel c(0, &loop);
  c.enable_read(true);
  c.enable_read(false);
}

TEST_CASE("write closed fd") {
  spdlog::set_level(spdlog::level::info);
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  CHECK(ret != -1);
  int local_fd = fds[0];
  int peer_fd = fds[1];
  CHECK(local_fd != 0);
  CHECK(peer_fd != 0);

  event_loop loop(-1);
  channel local(local_fd, &loop);
  channel peer(peer_fd, &loop);
  close(peer_fd);
  local.write("hello");
  // default on_error
  loop.wait();
}
TEST_CASE("handle_read") {
  std::atomic<bool> stop_wait{false};
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  CHECK(ret != -1);
  int local = fds[0];
  int peer = fds[1];
  CHECK(local != 0);
  CHECK(peer != 0);

  const char *send_msg = "hello";
  event_loop loop(-1);
  channel chan(local, &loop);
  chan.set_read_handler([send_msg](::malice::base::buffer &read_buf) {
    std::string msg = read_buf.take_all_as_string();
    CHECK(msg == send_msg);
  });
  chan.set_close_handler([&stop_wait](channel *chan, channel::close_type ct) {
    CHECK(ct == channel::close_type::eof);
    CHECK(chan->buffer_is_empty());
    close(chan->get_fd());
    stop_wait = true;
  });
  chan.enable_read(true);

  channel peer_chan(peer, &loop);
  peer_chan.set_write_finish_handler(
      [](channel *chan) { close(chan->get_fd()); });
  peer_chan.write(send_msg);
  while (!stop_wait) {
    loop.wait();
  }
  CHECK(true);
}
TEST_CASE("call default_on_close and write_buf full") {
  event_loop loop(-1);
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  CHECK(ret != -1);
  int local_fd = fds[0];
  int peer_fd = fds[1];
  CHECK(local_fd != 0);
  CHECK(peer_fd != 0);
  channel local(local_fd, &loop);
  channel peer(peer_fd, &loop);
  local.enable_read(true);

  peer.set_write_finish_handler([](channel *chan) { close(chan->get_fd()); });
  std::string msg(1024, 'c');
  int read_count = 0;
  local.set_read_handler([&msg, &read_count](::malice::base::buffer &buf) {
    read_count++;
    if (read_count == 1) {
      CHECK(buf.readable_size() == msg.size());
    } else if (read_count == 2) {
      CHECK(buf.readable_size() == msg.size() * 2);
      buf.take_all();
    }
  });
  peer.write(msg);
  peer.write(msg);
  CHECK(!peer.write_buffer_empty());
  loop.wait();
  CHECK(peer.write_buffer_empty());
  CHECK(local.read_buffer_empty());
  CHECK(read_count == 0);
  loop.wait();
  CHECK(read_count == 1);
  CHECK(!local.read_buffer_empty());
  loop.wait();
  CHECK(read_count == 2);
  // wait to close
  loop.wait();
}
