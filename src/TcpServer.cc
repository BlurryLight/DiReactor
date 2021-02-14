//
// Created by BlurryLight on 2021/2/14.
//

#include "TcpServer.hh"
#include "Acceptor.hh"
#include "EventLoop.hh"
#include "InetAddress.hh"
#include "SocketUtils.hh"
#include "TcpConnection.hh"

using namespace PD;
TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(CHECK_NOT_NULL(loop)), name_(listenAddr.to_host_port_str()),
      acceptor_(std::make_unique<Acceptor>(loop, listenAddr)), started_(false),
      nextConnId_(1) {
  acceptor_->set_new_conn_callback(
      [this](int sockfd, const InetAddress &peerAddr) {
        newConnection(sockfd, peerAddr);
      });
}
TcpServer::~TcpServer() {}
void TcpServer::start() {
  if (!started_) {
    started_ = true;
  }
  if (!acceptor_->isListening()) {
    loop_->run_in_loop([this]() { acceptor_->listen(); });
  }
}
void TcpServer::setConnectionCallback(const ConnectionCallback &cb) {
  conn_cb_ = cb;
}
void TcpServer::setMessageCallback(const MessageCallback &cb) { msg_cb_ = cb; }
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  loop_->assert_in_thread();
  auto str = "#" + std::to_string(nextConnId_++);
  auto connName = name_ + str;
  spdlog::info("TcpServer::newConnection [{}] new connection is [{}] from {}",
               name_, connName, peerAddr.to_host_port_str());
  InetAddress localAddr(getLocalAddr(sockfd));
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      loop_, connName, sockfd, localAddr, peerAddr);
  connections_[connName] = conn; // ref + 1

  conn->setConnectionCallback(conn_cb_);
  conn->setMessageCallback(msg_cb_);
  conn->connectEstablished();
}
