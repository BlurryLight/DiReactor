//
// Created by BlurryLight on 2021/1/27.
//
#pragma once

#include "utils.hh"
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
namespace PD {
// forward declaration
class Channel;
class Poller;

class EventLoop : public Noncopyable {
public:
  EventLoop();
  ~EventLoop();
  void run();
  [[nodiscard]] bool is_in_thread() const;
  void assert_in_thread() const;
  void update_channel(Channel *channel);
  void quit();

#ifndef GTEST_UNIT_TEST
  // expose all private members to GTEST
private:
#endif
  // abort when loop is not in that it was created
  void abort_not_in_thread() const;
  const std::thread::id tid_;
  std::atomic<bool> looping_;
  std::atomic<bool> quit_;
  std::unique_ptr<Poller> poller_;
  // EventLoop has no ownership of channel!
  std::vector<Channel *> active_channels_;
};
} // namespace PD
