//
// Created by BlurryLight on 2021/2/14.
//

#pragma once
#include "Callbacks.hh"
#include "InetAddress.hh"
#include "utils.hh"
#include <any>
#include <memory>
namespace PD {
class Channel;
class EventLoop;
class Socket;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class TcpConnection : public Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                const InetAddress &localAddr, const InetAddress &peerAddr);
  ~TcpConnection();

  EventLoop *getLoop() const { return loop_; }
  std::string name() const { return name_; }
  const InetAddress &localAddress() const { return localAddr_; }
  const InetAddress &peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  void setConnectionCallback(const ConnectionCallback &cb) { conn_cb_ = cb; }

  void setMessageCallback(const MessageCallback &cb) { msg_cb_ = cb; }

  void connectEstablished(); // assert only once

private:
  enum State { kConnecting, kConnected };
  void setState(State e) { state_ = e; }
  void handleRead();
  EventLoop *loop_;
  std::atomic<State> state_;
  std::string name_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback conn_cb_;
  MessageCallback msg_cb_;
};

} // namespace PD
