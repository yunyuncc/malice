#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/event_loop_thread.hpp"
#include <chrono>
#include <doctest/doctest.h>
#include <future>

using namespace malice::event;

TEST_CASE("test event_loop_thread run") {
  auto main_id = std::this_thread::get_id();
  event_loop_thread ev_thread;
  ev_thread.start();
  std::promise<int> c_promise;
  std::future<int> c_future = c_promise.get_future();
  ev_thread.run([&main_id, &c_promise] {
    c_promise.set_value(233);
    auto run_id = std::this_thread::get_id();
    CHECK(main_id != run_id);
  });
  CHECK(c_future.get() == 233);
  ev_thread.stop();
}
