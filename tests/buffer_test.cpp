#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "base/buffer.hpp"
using namespace malice::base;

TEST_CASE("create a buffer") {
  buffer buf;
  CHECK(buf.writable_size() == buffer::init_size);
  CHECK(buf.readable_size() == 0);
  CHECK(buf.prependable_size() == buffer::prepend_size);
}
TEST_CASE("make space") {
  buffer buf;
  size_t cap = buf.cap();
  buf.ensure_space(1024);
  CHECK(cap == buf.cap());
  buf.ensure_space(1500);
  CHECK(buf.writable_size() >= 1500);
  buf.ensure_space(2500);
  CHECK(buf.writable_size() >= 2500);
  buf.ensure_space(12500);
  CHECK(buf.writable_size() >= 12500);
}
