#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event/channel.hpp"
#include <sys/socket.h>
#include <sys/types.h>
using namespace malice::event;
TEST_CASE("create  a channel") {
  event_loop loop(-1);
  channel c(0, &loop);
  c.enable_read(true);
  c.enable_read(false);
  c.enable_write(true);
  c.enable_write(false);
}
TEST_CASE("handle_read") {
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  CHECK(ret != -1);
  int local = fds[0];
  int peer = fds[1];
  CHECK(local != 0);
  CHECK(peer != 0);

  event_loop loop(-1);
  channel chan(local, &loop);
  chan.enable_read(true);
  const char *buf = "hello";
  write(peer, buf, strlen(buf));
  loop.wait();
  CHECK(true);
}
