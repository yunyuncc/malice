#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "event/event_loop.hpp"
#include <chrono>
#include <doctest/doctest.h>
#include <future>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
using namespace malice::event;

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
  CHECK(ret != -1);

  int local = fds[0];
  int peer = fds[1];
  CHECK(local != 0);
  CHECK(peer != 0);

  std::string msg = "hello";

  std::shared_ptr<event> ev = std::make_shared<event>(local, EPOLLIN);
  ev->set_handler(EPOLLIN, [&local, &msg](event *e) {
    CHECK(e->get_fd() == local);
    char buf[10] = {0};
    auto s = read(e->get_fd(), buf, sizeof(buf));
    CHECK(s == msg.size());
    CHECK(msg == buf);
    cout << "recv:" << buf << endl;
    close(local);
  });

  event_loop ev_loop(-1);
  ev_loop.add_event(ev.get());

  std::async([&peer, &msg] {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
    auto s = write(peer, msg.data(), msg.size());
    CHECK(s == msg.size());
    cout << "write:" << msg << endl;
    close(peer);
  });
  ev_loop.wait();
}
//添加event到event_loop失败
TEST_CASE("add event fail") {
  event_loop ev_loop(100);
  auto ev = std::make_shared<event>(-1, EPOLLIN);
  try {
    ev_loop.add_event(ev.get());
  } catch (const add_event_fail &e) {
    std::string msg = e.what();
    CHECK(msg == "Bad file descriptor");
  }
}
//修改event成功和失败
TEST_CASE("mod event fail") {
  event_loop loop(100);
  auto ev = std::make_shared<event>(0, EPOLLIN);
  loop.add_event(ev.get());
  int flag = ev->get_flag();
  flag |= EPOLLOUT;
  ev->set_flag(flag);
  loop.mod_event(ev.get());

  auto ev2 = std::make_shared<event>(1, EPOLLIN);
  try {
    loop.mod_event(ev2.get());
  } catch (const mod_event_fail &e) {
    std::string msg = e.what();
    CHECK(msg == "No such file or directory event_loop::mod_event");
  }
}
void handle_alarm(int sig, siginfo_t *, void *) {
  CHECK(SIGALRM == sig);
  return;
}
TEST_CASE("wait event error") {
  event_loop loop(-1);
  auto ev = std::make_shared<event>(0, EPOLLIN);
  loop.add_event(ev.get());
  struct sigaction sigact;
  sigaction(SIGALRM, nullptr, &sigact);
  sigact.sa_sigaction = handle_alarm;
  sigaction(SIGALRM, &sigact, nullptr);
  ::alarm(1);
  loop.wait();
}

// TODO: event_loop 怎么处理已经析构掉的event
