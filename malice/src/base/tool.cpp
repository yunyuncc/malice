#include "base/tool.hpp"
#include <mutex>
#include <sys/syscall.h>
#include <unistd.h>

namespace malice::base {

std::mutex cached_tid_mutex;
__thread pid_t cached_tid = 0;
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

} // namespace malice::base
