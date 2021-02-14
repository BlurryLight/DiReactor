//
// Created by BlurryLight on 2021/2/14.
//

#include "TcpConnection.hh"
#include "Channel.hh"
#include "EventLoop.hh"
#include "Socket.hh"
#include <unistd.h>
using namespace PD;

static int flag = 0; // flag to check established
TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(CHECK_NOT_NULL(loop)), name_(name), state_(kConnecting),
      socket_(std::make_unique<Socket>(sockfd)),
      channel_(std::make_unique<Channel>(loop, sockfd)), localAddr_(localAddr),
      peerAddr_(peerAddr) {
  channel_->set_read_callback([this]() { handleRead(); });
}
TcpConnection::~TcpConnection() {
  spdlog::info("TcpConnection on sockfd {} dtor!", socket_->fd());
}
void TcpConnection::connectEstablished() {
  assert(flag == 0);
  flag = 1;
  loop_->assert_in_thread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enable_reading();
  assert(conn_cb_);
  conn_cb_(shared_from_this());
}
void TcpConnection::handleRead() {
  char buf[65536]; // not good
  ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
  spdlog::info("TcpConnection::Handle read(), {} size read", n);
  msg_cb_(shared_from_this(), buf, n);
}
