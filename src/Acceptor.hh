//
// Created by BlurryLight on 2021/2/11.
//

#pragma once
#include "Callbacks.hh"
#include "Channel.hh"
#include "Socket.hh"
#include "utils.hh"
#include <atomic>

namespace PD {

class InetAddress;
class EventLoop;
class Socket;
class Channel;
class Acceptor : public Noncopyable {
public:
  using NewConnCallbackFunc =
      std::function<void(int sockfd, const InetAddress &addr)>;
  Acceptor(EventLoop *loop, const InetAddress &listen_addr);
  void set_new_conn_callback(const NewConnCallbackFunc &cb);
  bool isListening() const { return listening_; }
  void listen();

private:
  void handle_read();
  EventLoop *loop_;
  Socket accept_socket_;
  Channel accept_socket_channel_;
  NewConnCallbackFunc new_conn_cb_;
  std::atomic<bool> listening_;
};

} // namespace PD
