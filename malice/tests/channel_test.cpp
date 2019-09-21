#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event/channel.hpp"
#include <atomic>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
using namespace malice::event;
TEST_CASE("create  a channel") {
  event_loop loop(-1);
  channel c(0, &loop);
  c.enable_read(true);
  c.enable_read(false);
  c.enable_write(true);
  c.enable_write(false);
}

TEST_CASE("write closed fd") {
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
  local.set_error_handler([](int err, const std::string &err_msg) {
    CHECK(err == EPIPE);
    CHECK(err_msg == "channel::handle_write");
    throw std::string("test throw a unknown exception of string type");
  });
  local.write("hello");
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
  chan.set_close_handler([&stop_wait](channel *chan) {
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
