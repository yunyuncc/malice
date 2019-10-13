#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/tool.hpp"
#include <doctest/doctest.h>
#include <thread>
using namespace malice::base;
TEST_CASE("error str") {
  errno = EACCES;
  auto e_str = errno_str();
  CHECK(e_str == "Permission denied");
  errno = 0;
  e_str = errno_str();
  CHECK(e_str == "Success");
}

TEST_CASE("test gettid") {
  pid_t pid = getpid();
  CHECK(pid == gettid());
  CHECK(gettid() == gettid());
  std::thread th([] {
    CHECK(getpid() != gettid());
    CHECK(gettid() == gettid());
  });
  std::thread th2([] {
    CHECK(getpid() != gettid());
    CHECK(gettid() == gettid());
  });
  th.join();
  th2.join();
}
