#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/event_channel.hpp"
#include <doctest/doctest.h>
#include <future>
using spdlog::info;
using namespace malice::event;
TEST_CASE("test weakup") {
  auto loop = std::make_shared<event_loop>(-1);
  event_channel ev_chan(loop, [](uint64_t num) { CHECK(num == 1); });
  ev_chan.notify();
  loop->wait();
  // has been weakup by ev_chan.notify
  CHECK(true);
}
TEST_CASE("test pass a event") {
  auto loop = std::make_shared<event_loop>(-1);
  event_channel ev_chan(loop, [](uint64_t ev_num) { CHECK(ev_num == 233); });
  std::async([&ev_chan] { ev_chan.notify(233); });
  loop->wait();
}
TEST_CASE("test notify fail") {
  auto loop = std::make_shared<event_loop>(-1);
  event_channel ev_chan(loop);
  info("test 01");
  ev_chan.notify(UINT64_MAX - 1);
  info("test 02");
  ev_chan.notify();
  info("test 03");
  loop->wait();
}
