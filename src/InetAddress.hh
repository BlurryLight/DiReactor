//
// Created by BlurryLight on 2021/2/12.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <netinet/in.h> //struct sockaddr_in
#include <string>
namespace PD {
// POD wrapper of sockaddr_in
class InetAddress {
public:
  // port should be host-endian
  explicit InetAddress(uint16_t portH);
  // ip should be presentation format: 1.1.1.1
  // port: host-endian
  explicit InetAddress(const std::string &ipH, uint16_t portH);
  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

  // print host:port
  std::string to_host_port_str() const;

  InetAddress() = default;
  ~InetAddress() = default;
  const sockaddr_in &get_sockaddr_struct() const { return addr_; };
  void set_sockaddr_struct(const struct sockaddr_in &addr) { addr_ = addr; }

private:
  struct sockaddr_in addr_;
};

} // namespace PD
