#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event/channel.hpp"
using namespace malice::event;
TEST_CASE("create  a channel") {
  event_loop loop(-1);
  channel c(0, &loop);
  c.enable_read(true);
  c.enable_read(false);
  c.enable_write(true);
  c.enable_write(false);
}
