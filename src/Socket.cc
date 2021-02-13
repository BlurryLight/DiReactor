//
// Created by BlurryLight on 2021/2/11.
//

#include "Socket.hh"
#include "InetAddress.hh"
#include "SocketUtils.hh"
using namespace PD;
Socket::Socket(int sockfd) : sockfd_(sockfd) {}
Socket::~Socket() { close(sockfd_); }
int Socket::fd() const { return sockfd_; }
void Socket::bind_address(const InetAddress &addr) {
  bindOrDie(sockfd_, addr.get_sockaddr_struct());
}
void Socket::listen() { listenOrDie(sockfd_); }
int Socket::accept(InetAddress *peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = PD::accept(sockfd_, &addr);
  // connfd may < 0 because of EAGAIN error
  if (connfd > 0) {
    peeraddr->set_sockaddr_struct(addr);
  }
  return connfd;
}
void Socket::enableReuseAddr(bool enabled) {
  int opt = enabled ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
  if (ret < 0) {
    Log_Abort("enableReuseAddr failed!");
  }
}
