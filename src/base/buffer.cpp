#include "base/buffer.hpp"
#include <algorithm>
#include <cassert>
namespace malice::base {

const size_t buffer::prepend_size;
const size_t buffer::init_size;

buffer::buffer(size_t size) : buf(size + prepend_size) {
  reset();
  assert(readable_size() == 0);
  assert(writable_size() == init_size);
}

void buffer::ensure_writable(size_t len) {
  reset_if_no_readable();
  if (writable_size() >= len) {
    return;
  }
  size_t old_size = prependable_size() + readable_size() + writable_size();
  size_t new_size1 = old_size * 2;
  size_t new_size2 = prependable_size() + readable_size() + len + init_size;
  size_t new_size = std::max(new_size1, new_size2);
  buf.resize(new_size);
}

} // namespace malice::base
