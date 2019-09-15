#pragma once
#include "base/tool.hpp"
#include <cassert>
#include <string.h>
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
  void has_writen(size_t len) { write_idx += len; }
  bool is_clean() const {
    return (read_idx == prepend_size) && (write_idx == prepend_size);
  }

  void append(const char *data, size_t len) {
    ensure_writable(len);
    std::copy(data, data + len, begin_write());
    has_writen(len);
  }
  void append(const void *data, size_t len) {
    append(static_cast<const char *>(data), len);
  }
  void append(const std::string &str) { append(str.data(), str.size()); }
  //定义两种读
  // read:可以多次读
  // take:会取走数据，take后的内存会被回收利用
  void has_take(size_t len) {
    assert(len <= readable_size());
    read_idx += len;
    reset_if_no_readable();
  }
  std::string take_all_as_string() { return take_as_string(readable_size()); }
  std::string take_as_string(size_t len) {
    assert(len <= readable_size());
    std::string res(begin_read(), begin_read() + len);
    has_take(len);
    return res;
  }
  char *begin_write() { return begin() + prependable_size() + readable_size(); }
  char *begin_read() { return begin() + prependable_size(); }
  //确保writable_size >= len,在向buffer中写入数据的时候调用，保证写入成功
  void ensure_writable(size_t len);
  size_t cap() { return buf.size(); }

private:
  char *begin() { return &(*buf.begin()); }
  std::vector<char> buf;
  size_t read_idx;
  size_t write_idx;
};

CREATE_NEW_EXCEPTION(buffer_full);

} // namespace malice::base
