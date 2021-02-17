//
// Created by BlurryLight on 2021/2/14.
//

#include "TcpConnection.hh"
#include "Channel.hh"
#include "EventLoop.hh"
#include "Socket.hh"
#include "SocketUtils.hh"
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
  channel_->set_write_callback([this]() { handleRead(); });
  channel_->set_close_callback([this]() { handleClose(); });
  channel_->set_error_callback([this]() { handleError(); });
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
  if (n > 0) {
    msg_cb_(shared_from_this(), buf, n);
  } else if (n == 0) {
    handleClose();
  } else {
    handleError();
  }
}
void TcpConnection::connectDestroyed() {
  assert(flag == 1);
  flag = 0;
  loop_->assert_in_thread();
  assert(state_ == kConnected);
  setState(kDisconnected);
  channel_->disable_all();
  assert(conn_cb_);
  conn_cb_(shared_from_this());
  loop_->removeChannel(channel_.get());
}
void TcpConnection::handleClose() {
  loop_->assert_in_thread();
  assert(state_ == kConnected);
  channel_->disable_all();
  close_cb_(shared_from_this());
}
void TcpConnection::handleError() {
  auto err = getSocketError(socket_->fd());
  char buf[128];
  spdlog::error("TcpConnection::handleError {}, {}", name(),
                strerror_r(err, buf, sizeof buf));
}
