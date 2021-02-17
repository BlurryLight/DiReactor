//
// Created by BlurryLight on 2021/2/14.
//

#pragma once
#include "Callbacks.hh"
#include "utils.hh"
#include <map>
#include <memory>
namespace PD {
class EventLoop;
class InetAddress;
class Acceptor;
class TcpServer : public Noncopyable {
public:
  TcpServer(EventLoop *loop, const InetAddress &listenAddr);
  ~TcpServer() = default;

  void start();

  // NOT thread-safe
  void setConnectionCallback(const ConnectionCallback &cb);
  // NOT thread-safe
  void setMessageCallback(const MessageCallback &cb);

private:
  // assert in loop
  // spawn a TcpConnection object
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);

  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
  EventLoop *loop_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  ConnectionCallback conn_cb_;
  MessageCallback msg_cb_;
  std::atomic<bool> started_;
  int nextConnId_; // assert in loop
  ConnectionMap connections_;
};
}; // namespace PD
