#pragma once
#include <errno.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/types.h>

#ifndef CREATE_NEW_EXCEPTION
#define CREATE_NEW_EXCEPTION(name)                                             \
  class name : public std::runtime_error {                                     \
  public:                                                                      \
    using std::runtime_error::runtime_error;                                   \
  }
#endif
namespace malice::base {

inline std::string errno_str(int e) {
  char buf[512];
  std::string e_str = strerror_r(e, buf, sizeof(buf));
  return e_str;
}

inline std::string errno_str() { return errno_str(errno); }

pid_t gettid();

} // namespace malice::base
