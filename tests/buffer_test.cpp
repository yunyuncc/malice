#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "base/buffer.hpp"
#include <algorithm>
using namespace malice::base;

TEST_CASE("create a buffer") {
  buffer buf;
  CHECK(buf.writable_size() == buffer::init_size);
  CHECK(buf.readable_size() == 0);
  CHECK(buf.prependable_size() == buffer::prepend_size);
}
TEST_CASE("ensure space") {
  buffer buf;
  size_t cap = buf.cap();
  buf.ensure_writable(1024);
  CHECK(cap == buf.cap());
  buf.ensure_writable(1500);
  CHECK(buf.writable_size() >= 1500);
  buf.ensure_writable(2500);
  CHECK(buf.writable_size() >= 2500);
  buf.ensure_writable(12500);
  CHECK(buf.writable_size() >= 12500);
}

TEST_CASE("read write") {
  buffer buf;
  std::string hello = "hello";
  buf.ensure_writable(hello.size());
  std::copy(hello.begin(), hello.end(), buf.begin_write());
  buf.has_writen(hello.size());
  CHECK(buf.readable_size() == hello.size());
  CHECK(buf.writable_size() == (buffer::init_size - hello.size()));
  std::string msg(buf.begin_read(), buf.begin_read() + buf.readable_size());
  CHECK(msg == hello);
  CHECK(buf.readable_size() == hello.size());
  CHECK(buf.prependable_size() == buffer::prepend_size);

  auto taken = buf.take_as_string(3);
  CHECK(taken == "hel");
  CHECK(buf.readable_size() == (hello.size() - taken.size()));
  CHECK(buf.prependable_size() == (buffer::prepend_size + taken.size()));
  taken += buf.take_all_as_string();
  CHECK(taken == hello);
  CHECK(buf.readable_size() == 0);
  CHECK(buf.is_clean());

  buf.append(hello);
  CHECK(buf.readable_size() == hello.size());
  auto taken2 = buf.take_all_as_string();
  CHECK(taken2 == hello);
  CHECK(buf.is_clean());
}
