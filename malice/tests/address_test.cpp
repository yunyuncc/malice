#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "net/address.hpp"
#include <algorithm>
#include <doctest/doctest.h>
using namespace spdlog;
using namespace malice::net;
TEST_CASE("test ai_family_str") {
  CHECK(ai_family_str(AF_INET) == "AF_INET");
  CHECK(ai_family_str(AF_INET6) == "AF_INET6");
  CHECK(ai_family_str(AF_UNSPEC) == "AF_UNSPEC");
}
TEST_CASE("test ai_socktype_str") {
  CHECK(ai_socktype_str(SOCK_STREAM) == "SOCK_STREAM");
  CHECK(ai_socktype_str(SOCK_DGRAM) == "SOCK_DGRAM");
}
TEST_CASE("test ai_protocol_str") {
  CHECK(ai_protocol_str(IPPROTO_IP) == "IPPROTO_IP");
  CHECK(ai_protocol_str(IPPROTO_IPV6) == "IPPROTO_IPV6");
  CHECK(ai_protocol_str(IPPROTO_UDP) == "IPPROTO_UDP");
  CHECK(ai_protocol_str(IPPROTO_TCP) == "IPPROTO_TCP");
}
TEST_CASE("test ai_flags_str") {
  CHECK(ai_flags_str(AI_PASSIVE) == "AI_PASSIVE|");
  CHECK(ai_flags_str(AI_CANONNAME) == "AI_CANONNAME|");
  CHECK(ai_flags_str(AI_NUMERICHOST) == "AI_NUMERICHOST|");
  CHECK(ai_flags_str(AI_V4MAPPED) == "AI_V4MAPPED|");
  CHECK(ai_flags_str(AI_ALL) == "AI_ALL|");
  CHECK(ai_flags_str(AI_NUMERICSERV) == "AI_NUMERICSERV|");
  CHECK(ai_flags_str(AI_CANONNAME | AI_NUMERICHOST) ==
        "AI_CANONNAME|AI_NUMERICHOST|");
}

TEST_CASE("test address ipv4") {
  address addr("127.0.0.1", 10005);
  CHECK(sockaddr_str(addr.get_sockaddr()) == "127.0.0.1 10005");
}
TEST_CASE("test address ipv6") {
  std::string addr_str = "ABCD:EF01:2345:6789:ABCD:EF01:2345:6789";
  address addr(addr_str, 80);
  std::transform(addr_str.begin(), addr_str.end(), addr_str.begin(), tolower);
  CHECK(sockaddr_str(addr.get_sockaddr()) == (addr_str + " 80"));
}
TEST_CASE("test sockaddr_str unspec") {
  struct sockaddr addr;
  addr.sa_family = AF_UNSPEC;
  CHECK(sockaddr_str(&addr) == "AF_UNSPEC");
}
