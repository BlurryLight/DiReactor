//
// Created by BlurryLight on 2021/1/29.
//

#include "Poller.hh"
#include "Channel.hh"
#include "EventLoop.hh"
#include "poll.h"
#include "spdlog/spdlog.h"
using namespace PD;
void Poller::assert_in_loop_thread() { ownerloop_->assert_in_thread(); }
Poller::Poller(EventLoop *loop) : ownerloop_(loop) {}
time_point Poller::poll(int timeoutMS, std::vector<Channel *> &activeChannels) {
  activeChannels.clear();
  int num_active = ::poll(pollfds_.data(), pollfds_.size(), timeoutMS);
  auto now = std::chrono::system_clock::now();
  if (num_active > 0) {
    spdlog::info("Poller::poll: {} events happens!", num_active);
    for (auto it = pollfds_.cbegin(); it != pollfds_.cend() && num_active > 0;
         ++it) {
      if (it->revents > 0) {
        --num_active;
        auto map_it = channels_.find(it->fd);
        assert(map_it != channels_.end());
        auto channel = map_it->second;
        assert(channel->fd() == it->fd);
        channel->set_revents(it->revents);
        activeChannels.push_back(channel);
      }
    }
  } else if (num_active == 0) {
    spdlog::info("Poller::poll: Nothing happens,timeout!");
  } else {
    spdlog::error("Poller::poll: Error happens! {} ", strerror(errno));
  }
  return now;
}
void Poller::update_channel(Channel *channel) {
  spdlog::info("Poller::update_channel:"
               "fd:{}, events:{}",
               channel->fd(), channel->events());
  if (channel->index() < 0) {
    // new channel has not been in poller
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.events = static_cast<short>(channel->events());
    pfd.fd = channel->fd();
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size()) - 1);
    channels_[channel->fd()] = channel;
  } else {
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->index()] == channel);
    int index = channel->index();
    assert(index >= 0 && (index <= pollfds_.size() - 1));
    auto &pfd = pollfds_[index];
    assert(pfd.fd == channel->fd() || pfd.fd == (-channel->fd() - 1));
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->is_none_event()) {

      // man 2 poll
      // if fd is negative, ::poll will ignore the fd
      pfd.fd = -channel->fd() - 1;
    }
  }
}
