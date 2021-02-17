//
// Created by BlurryLight on 2021/2/11.
//

#include "SocketUtils.hh"
#include "Socket.hh"
#include "spdlog/spdlog.h"
#include "unistd.h"
#include <arpa/inet.h>
#include <endian.h>

static auto sockaddr_cast(const struct sockaddr_in *addr) {
  return reinterpret_cast<const struct sockaddr *>(addr);
}

static auto sockaddr_cast(struct sockaddr_in *addr) {
  return reinterpret_cast<struct sockaddr *>(addr);
}
uint64_t PD::host_to_network64(uint64_t host64) { return htobe64(host64); }
uint32_t PD::host_to_network32(uint32_t host32) { return htobe32(host32); }
uint16_t PD::host_to_network16(uint16_t host16) { return htobe16(host16); }
uint64_t PD::net_to_host64(uint64_t net64) { return be64toh(net64); }
uint32_t PD::net_to_host32(uint64_t net32) { return be32toh(net32); }
uint16_t PD::net_to_host16(uint64_t net16) { return be16toh(net16); }
PD::Socket PD::createNonblocking() {

  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP); // protocol: 0 is also ok
  if (sockfd < 0) {
    Log_Abort("socket create failed!");
  }
  return PD::Socket(sockfd);
}
void PD::close(int sockfd) {
  assert(sockfd > 0);
  if (::close(sockfd) < 0) {
    Log_Abort("close fd failed!");
  }
}
// addr should be zeroed!
void PD::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  assert(addr->sin_port == 0);
  assert(addr->sin_family == 0);
  assert(addr->sin_addr.s_addr == 0);
  assert(ip);
  addr->sin_family = AF_INET;
  addr->sin_port = host_to_network16(port);
  int ret = inet_pton(AF_INET, ip, &addr->sin_addr.s_addr);
  if (ret <= 0) {
    Log_Abort("fromHostPort failed!ip:port {},{}", ip, port);
  }
}
void PD::toHostPort(char *buf, size_t size, const sockaddr_in &addr) {
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr.s_addr, host, sizeof host);
  uint16_t port = net_to_host16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}
void PD::listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    Log_Abort("listen failed!");
  }
}
int PD::accept(int sockfd, struct sockaddr_in *peeraddr) {
  socklen_t addr_len = sizeof *peeraddr;
  int connfd = accept4(sockfd, sockaddr_cast(peeraddr), &addr_len,
                       SOCK_NONBLOCK | SOCK_CLOEXEC);
  // error handling
  if (connfd < 0) {
    int saved_errno = errno;
    switch (saved_errno) {
      // small error
    case (EAGAIN):
      //    case (EWOULDBLOCK): // In Linux #define EWOULDBLOCK  EAGAIN
    case EINTR:
    case ECONNABORTED:
    case EMFILE: // process fd out of limit
      errno = saved_errno;
      break;
    default:
      Log_Abort("Fatal error when accept! errno: {}", saved_errno);
    }
  }
  return connfd;
}
void PD::bindOrDie(int sockfd, const sockaddr_in &addr) {
  int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
  if (ret < 0) {
    Log_Abort("bind failed!");
  }
}
struct sockaddr_in PD::getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);

  socklen_t addrlen = sizeof localaddr;
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    Log_Abort("getsockname failed!");
  }
  return localaddr;
}
int PD::getSocketError(int sockfd) {
  int optval = 0;
  socklen_t optlen = sizeof optval;
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  }
  return optval;
}
