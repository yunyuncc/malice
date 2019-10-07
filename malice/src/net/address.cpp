#include "net/address.hpp"
#include <cassert>
#include <map>
namespace malice::net {
std::string ai_family_str(int family) {
  static const std::map<int, std::string> ai_family_map{
      {AF_INET, "AF_INET"}, {AF_INET6, "AF_INET6"}, {AF_UNSPEC, "AF_UNSPEC"}};
  assert(ai_family_map.count(family) != 0);
  return ai_family_map.at(family);
}

std::string ai_socktype_str(int socktype) {
  static const std::map<int, std::string> ai_socktype_map{
      {SOCK_STREAM, "SOCK_STREAM"}, {SOCK_DGRAM, "SOCK_DGRAM"}};
  assert(ai_socktype_map.count(socktype) != 0);
  return ai_socktype_map.at(socktype);
}
std::string ai_protocol_str(int protocol) {
  static const std::map<int, std::string> ai_protocol_map{
      {IPPROTO_IP, "IPPROTO_IP"},
      {IPPROTO_IPV6, "IPPROTO_IPV6"},
      {IPPROTO_UDP, "IPPROTO_UDP"},
      {IPPROTO_TCP, "IPPROTO_TCP"}};
  assert(ai_protocol_map.count(protocol) != 0);
  return ai_protocol_map.at(protocol);
}
std::string ai_flags_str(int flag) {
  static const std::map<int, std::string> ai_flags_map{
      {AI_PASSIVE, "AI_PASSIVE"}, /* Socket address is intended for `bind'.  */
      {AI_CANONNAME, "AI_CANONNAME"},     /* Request for canonical name.  */
      {AI_NUMERICHOST, "AI_NUMERICHOST"}, /* Don't use name resolution.  */
      {AI_V4MAPPED, "AI_V4MAPPED"}, /* IPv4 mapped addresses are acceptable.  */
      {AI_ALL, "AI_ALL"}, /* Return IPv4 mapped and IPv6 addresses.  */
      {AI_NUMERICSERV, "AI_NUMERICSERV"} /* Don't use name resolution.  */
  };
  std::string res;
  for (auto [k, v] : ai_flags_map) {
    if (k & flag) {
      res += v;
      res += "|";
    }
  }
  return res;
}
address::address(const std::string &host, const std::string &port) {
  int ret = getaddrinfo(host.c_str(), port.c_str(), nullptr, &addr);
  if (ret != 0) {
    throw getaddrinfo_fail(std::string("getaddrinfo:") + std::to_string(ret));
  }
}

address::~address() {
  if (addr) {
    freeaddrinfo(addr);
    addr = nullptr;
  }
}

} // namespace malice::net
