#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include <doctest/doctest.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <string>
using namespace spdlog;
TEST_CASE("test log") {
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%Y-%m-%d-%H:%M:%S %e] [%P] <%t> [%-8l] %v");
  info("hello malice");
  error("some error happen err_num:{}", 1);
  warn("some warn happen err_num:{}", 2);
  debug("for debug");
  spdlog::set_level(spdlog::level::info);
  debug("for debug this should not log");
  critical("critical error happen");
  std::string buf{"123456781234567812345678123456781234567812345678"};
  info("{}", spdlog::to_hex(buf));
}
