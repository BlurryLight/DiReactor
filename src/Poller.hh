//
// Created by BlurryLight on 2021/1/29.
//

#pragma once
#include "utils.hh"
#include <chrono>
#include <map>
#include <vector>

using time_point = std::chrono::system_clock::time_point;
// fwd declaration
struct pollfd;

namespace PD {
// forward declaration
class EventLoop;
class Channel;
// Core class
// IO multiplexer with poll
class Poller : public Noncopyable {
public:
  Poller(EventLoop *loop);
  ~Poller() = default;
  time_point poll(int timeoutMS, std::vector<Channel *> &activeChannels);
  void update_channel(Channel *channel);
  void assert_in_loop_thread();

private:
  EventLoop *ownerloop_;
  std::vector<pollfd> pollfds_;

  using fd_t = int;
  // map active fd to channel
  std::map<fd_t, Channel *> channels_;
};

} // namespace PD
