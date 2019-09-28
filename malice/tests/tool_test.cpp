#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/tool.hpp"
#include <doctest/doctest.h>
using namespace malice::base;
TEST_CASE("error str") {
  errno = EACCES;
  auto e_str = errno_str();
  CHECK(e_str == "Permission denied");
  errno = 0;
  e_str = errno_str();
  CHECK(e_str == "Success");
}
