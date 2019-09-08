#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event/event_loop.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;
using namespace malice::event;
TEST_CASE("create_event_loop_fail") {
  try {
    throw create_event_loop_fail("for test");
    CHECK(false);
  } catch (const create_event_loop_fail &e) {
    CHECK(true);
    std::string msg = e.what();
    CHECK(msg == "for test");
  } catch (const std::runtime_error &e) {
    CHECK(false);
  }
}
TEST_CASE("timeout event_loop") {
  event_loop ev_loop(100);
  int count = 0;
  ev_loop.set_timeout_handler([&count] { count++; });
  auto beg = std::chrono::steady_clock::now();
  ev_loop.wait();
  auto end = std::chrono::steady_clock::now();
  int dur_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count();
  CHECK(dur_ms >= 100);
}
TEST_CASE("wait event") {
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  int local = fds[0];
  std::string msg = "hello";
  int peer = fds[1];
  CHECK(ret != -1);
  CHECK(local != 0);
  CHECK(peer != 0);
  std::shared_ptr<event> ev = std::make_shared<event>(local, EPOLLIN);
  ev->set_handler(EPOLLIN, [&local, &msg](event *e) {
    CHECK(e->get_fd() == local);
    char buf[10] = {0};
    auto s = read(e->get_fd(), buf, sizeof(buf));
    CHECK(s == msg.size());
    CHECK(msg == buf);
    cout << "recv:" << buf << endl;
  });
  event_loop ev_loop(-1);
  ev_loop.add_event(ev.get());
  std::async([&peer, &msg] {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
    auto s = write(peer, msg.data(), msg.size());
    CHECK(s == msg.size());
    cout << "write:" << msg << endl;
  });
  ev_loop.wait();
}
// TODO: event_loop 怎么处理已经析构掉的event
