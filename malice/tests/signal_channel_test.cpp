#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/signal_channel.hpp"
#include <doctest/doctest.h>
#include <future>
#include <sys/types.h>
#include <unistd.h>
using namespace malice::event;
TEST_CASE("only on signal_channel can be created") {
  event_loop loop(-1);
  signal_channel chan1(&loop);
  try {
    signal_channel chan2(&loop);
  } catch (const mult_signal_channel &) {
    CHECK(true);
  }
}
TEST_CASE("set signal handler fail") {
  event_loop loop(-1);
  signal_channel chan(&loop);
  try {
    chan.set_signal_handler(-1, nullptr);
  } catch (const set_signal_handler_bad_param &) {
    CHECK(true);
  }
  try {
    chan.set_signal_handler(SIGKILL, nullptr);
  } catch (const set_signal_handler_bad_param &) {
    CHECK(true);
  }
}
TEST_CASE("got signal without a handler deal it") {
  event_loop loop(-1);
  signal_channel chan(&loop);
  std::async([] { kill(getpid(), SIGUSR1); });
  loop.wait();
}
TEST_CASE("ignore a signal") {
  spdlog::set_level(spdlog::level::debug);
  event_loop loop(-1);
  signal_channel chan(&loop);
  chan.ignore(SIGUSR1);
  std::async([] { kill(getpid(), SIGUSR1); });
  loop.wait();
}
TEST_CASE("handle SIGTERM and SIGQUIT") {
  event_loop loop(-1);
  signal_channel chan(&loop);
  chan.set_signal_handler(SIGTERM, [](const signalfd_siginfo &siginfo) {
    CHECK(siginfo.ssi_signo == SIGTERM);
  });
  chan.set_signal_handler(SIGQUIT, [](const signalfd_siginfo &siginfo) {
    CHECK(siginfo.ssi_signo == SIGQUIT);
  });
  std::async([] {
    kill(getpid(), SIGTERM);
    kill(getpid(), SIGQUIT);
  });
  loop.wait();
}
