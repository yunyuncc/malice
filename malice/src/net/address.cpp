#include "net/address.hpp"
#include <cassert>
#include <map>
#include <regex>

namespace malice::net {

static std::string addr_str4(const struct sockaddr_in *addr);
static std::string addr_str6(const struct sockaddr_in6 *addr);

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
address::address(const std::string &ip_, uint16_t port_)
    : ip(ip_), port(port_) {
  memset(&net_addr, 0, sizeof(struct sockaddr_storage));
  // TODO regex match ip , v4,v6
  const static std::regex ipv4_regex("\\d+\\.\\d+\\.\\d+\\.\\d+");
  if (regex_match(ip, ipv4_regex)) {
    struct sockaddr_in *p_addr = (struct sockaddr_in *)&net_addr;
    p_addr->sin_family = AF_INET;
    assert(inet_pton(AF_INET, ip.c_str(), &p_addr->sin_addr) == 1);
    p_addr->sin_port = htons(port);
  } else {
    struct sockaddr_in6 *p_addr = (struct sockaddr_in6 *)&net_addr;
    p_addr->sin6_family = AF_INET6;
    assert(inet_pton(AF_INET6, ip.c_str(), &p_addr->sin6_addr) == 1);
    p_addr->sin6_port = htons(port);
  }
}
std::string sockaddr_str(const struct sockaddr *addr) {
  if (addr->sa_family == AF_INET) {
    return addr_str4((const struct sockaddr_in *)addr);
  } else if (addr->sa_family == AF_INET6) {
    return addr_str6((const struct sockaddr_in6 *)addr);
  }
  return ai_family_str(AF_UNSPEC);
}
static std::string addr_str6(const struct sockaddr_in6 *addr) {
  assert(addr->sin6_family == AF_INET6);
  // struct in_addr
  char buf[INET6_ADDRSTRLEN] = {};
  inet_ntop(addr->sin6_family, (void *)(&addr->sin6_addr), buf, sizeof(buf));
  uint16_t port = ntohs(addr->sin6_port);
  std::string res(buf);
  res += " ";
  res += std::to_string(port);
  return res;
}
std::string addr_str4(const struct sockaddr_in *addr) {
  assert(addr->sin_family == AF_INET);
  // struct in_addr
  char buf[INET6_ADDRSTRLEN] = {};
  inet_ntop(addr->sin_family, (void *)(&addr->sin_addr), buf, sizeof(buf));
  uint16_t port = ntohs(addr->sin_port);
  std::string res(buf);
  res += " ";
  res += std::to_string(port);
  return res;
}

} // namespace malice::net
