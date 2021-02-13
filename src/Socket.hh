//
// Created by BlurryLight on 2021/2/11.
//

#pragma once
#include "utils.hh"
namespace PD {
class InetAddress;
// has ownership of socket fd
class Socket : public Noncopyable {
public:
  explicit Socket(int sockfd);
  ~Socket();
  int fd() const;
  // abort on error
  void bind_address(const InetAddress &addr);
  // abort onerror
  void listen();
  int accept(InetAddress *peeraddr);

  void enableReuseAddr(bool enabled);

private:
  const int sockfd_;
};

} // namespace PD
