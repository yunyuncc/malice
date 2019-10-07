#pragma once
#include "base/tool.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace malice::net {
class address {
public:
  address(const std::string &host, const std::string &port);
  ~address();
  std::string str() const {
    std::string res = "[\n";
    res += "flag:";
    res += std::to_string(addr->ai_flags);
    res += "\n";
    return res;
  }

private:
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
  struct addrinfo *addr = nullptr;
};
std::string ai_family_str(int family);
std::string ai_socktype_str(int socktype);
std::string ai_protocol_str(int protocol);
std::string ai_flags_str(int flag);
std::string sockaddr_str(struct sockaddr *addr); // TODO

CREATE_NEW_EXCEPTION(getaddrinfo_fail);
} // namespace malice::net
