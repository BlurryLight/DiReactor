//
// Created by BlurryLight on 2021/2/11.
//

#include "Acceptor.hh"
#include "EventLoop.hh"
#include "InetAddress.hh"
#include "Socket.hh"
#include "SocketUtils.hh"
using namespace PD;
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr)
    : loop_(loop), accept_socket_(createNonblocking()),
      accept_socket_channel_(loop, accept_socket_.fd()), listening_(false) {
  accept_socket_.enableReuseAddr(true);
  accept_socket_.bind_address(listen_addr);
  accept_socket_channel_.set_read_callback([this]() { this->handle_read(); });
}
void Acceptor::listen() {
  loop_->assert_in_thread();
  listening_ = true;
  accept_socket_.listen();
  accept_socket_channel_.enable_reading();
}
void Acceptor::handle_read() {
  loop_->assert_in_thread();
  InetAddress peerAddr;
  int connfd = accept_socket_.accept(&peerAddr);
  if (connfd > 0) { // has valid fd
    if (new_conn_cb_) {
      new_conn_cb_(connfd, peerAddr);
    } else {
      PD::close(connfd);
    }
  }
}
void Acceptor::set_new_conn_callback(const Acceptor::NewConnCallbackFunc &cb) {
  new_conn_cb_ = cb;
}
