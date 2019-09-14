#pragma once
#include "base/tool.hpp"
#include <vector>

namespace malice::base {

class buffer {
public:
  static const size_t prepend_size = 8;
  static const size_t init_size = 1024;
  buffer(size_t size = init_size);
  size_t writable_size() const { return buf.size() - write_idx; }
  size_t readable_size() const { return write_idx - read_idx; }
  size_t prependable_size() const { return read_idx; }
  void reset() {
    read_idx = prepend_size;
    write_idx = prepend_size;
  }
  void reset_if_no_readable() {
    if (readable_size() == 0) {
      reset();
    }
  }
  //确保writable_size >= len,在向buffer中写入数据的时候调用，保证写入成功
  void ensure_space(size_t len);
  size_t cap() { return buf.size(); }

private:
  std::vector<char> buf;
  size_t read_idx;
  size_t write_idx;
};

CREATE_NEW_EXCEPTION(buffer_full);

} // namespace malice::base
