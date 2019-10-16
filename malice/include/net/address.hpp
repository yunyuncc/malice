#pragma once
#include "base/tool.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
namespace malice::net {

std::string ai_family_str(int family);
std::string ai_socktype_str(int socktype);
std::string ai_protocol_str(int protocol);
std::string ai_flags_str(int flag);
std::string sockaddr_str(const struct sockaddr *addr);

class address {
public:
  address(const std::string &ip, uint16_t port); // suport ipv4 for now

  struct sockaddr *get_sockaddr() {
    return (struct sockaddr *)&net_addr;
  }

private:
  std::string ip;
  uint16_t port;
  struct sockaddr_storage net_addr;
  // typedef uint32_t in_addr_t;
  // struct in_addr
  //{
  //  in_addr_t s_addr;
  //};

  // struct sockaddr_in {
  //  sa_family_t sin_family;
  //  in_port_t sin_port;
  //  struct in_addr sin_addr;
  //  //pad
  //};

  // struct addrinfo {
  //    int              ai_flags;
  //    int              ai_family;
  //    int              ai_socktype;
  //    int              ai_protocol;
  //    socklen_t        ai_addrlen;
  //    struct sockaddr *ai_addr;
  //    char            *ai_canonname;
  //    struct addrinfo *ai_next;
  //};
  // std::string host_str;
};

} // namespace malice::net
