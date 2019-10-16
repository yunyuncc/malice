#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/timer.hpp"
#include <cmath>
#include <doctest/doctest.h>
using namespace spdlog;
using namespace malice::event;
using namespace std::literals::chrono_literals;
using namespace std::chrono;
TEST_CASE("test once timer") {
  event_loop loop(-1);
  timer t(&loop,
          [&loop](const std::string &name) {
            info("tick");
            loop.stop();
            CHECK(name == "test_timer_01");
          },
          10ms, 0ms, std::string("test_timer_01"));
  auto t1 = steady_clock::now();
  loop.loop();
  auto t2 = steady_clock::now();
  auto pass_us = duration_cast<microseconds>(t2 - t1).count();
  auto sub = abs(1.0 - pass_us / (10 * 1000));
  CHECK(sub < 0.01);
}

TEST_CASE("test period timer") {
  event_loop loop(-1);
  int c = 0;
  timer t(&loop,
          [&loop, &c](const std::string &name) {
            CHECK(name == "test_timer_02");
            info("tick 02");
            c++;
            if (c >= 10) {
              loop.stop();
              return;
            }
          },
          10ms, 10ms, std::string("test_timer_02"));
  auto t1 = steady_clock::now();
  loop.loop();
  auto t2 = steady_clock::now();
  CHECK(c == 10);
  auto pass_us = duration_cast<microseconds>(t2 - t1).count();
  auto sub = abs(1.0 - pass_us / (10 * 10 * 1000));
  CHECK(sub < 0.01);
}
TEST_CASE("test two timer") {
  event_loop loop(-1);
  int c1 = 0;
  int c2 = 0;
  auto on_time = [&c1, &c2, &loop](const std::string &name) {
    if (name == "timer01") {
      c1++;
    }
    if (name == "timer02") {
      c2++;
    }
    if (c1 + c2 >= 2) {
      loop.stop();
    }
  };
  timer timer1(&loop, on_time, 10ms, 0ms, std::string("timer01"));
  timer timer2(&loop, on_time, 200ms, 0ms, std::string("timer02"));
  auto t1 = steady_clock::now();
  loop.loop();
  auto t2 = steady_clock::now();
  auto pass_us = duration_cast<microseconds>(t2 - t1).count();
  auto sub = abs(1.0 - pass_us / (200 * 1000));
  CHECK(sub < 0.01);
  CHECK(c1 == 1);
  CHECK(c2 == 1);
}
