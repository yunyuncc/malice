#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
namespace malice::base {

void init_log(const std::string &log_path);
}
