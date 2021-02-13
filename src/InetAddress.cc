//
// Created by BlurryLight on 2021/2/12.
//

#include "InetAddress.hh"
#include "SocketUtils.hh"
#include <cstring>
using namespace PD;
static_assert(sizeof(InetAddress) == sizeof(sockaddr_in), "NOT POD");
InetAddress::InetAddress(uint16_t portH) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = host_to_network32(INADDR_ANY);
  addr_.sin_port = host_to_network16(portH);
}
InetAddress::InetAddress(const std::string &ipH, uint16_t portH) {

  bzero(&addr_, sizeof addr_);
  fromHostPort(ipH.c_str(), portH, &addr_);
}
std::string InetAddress::to_host_port_str() const {
  char buf[32];
  toHostPort(buf, sizeof buf, addr_);
  return buf;
}
