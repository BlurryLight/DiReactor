//
// Created by BlurryLight on 2021/1/27.
//
#pragma once

#include "Callbacks.hh"
#include "TimerProxy.hh"
#include "utils.hh"
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
namespace PD {
// forward declaration
class Channel;
class Poller;
class TimerProxy;
class TimerQueue;

using time_point = std::chrono::system_clock::time_point;
class EventLoop : public Noncopyable {
public:
  EventLoop();
  ~EventLoop();
  void run();
  [[nodiscard]] bool is_in_thread() const;
  void assert_in_thread() const;
  void update_channel(Channel *channel);
  void quit();

  TimerProxy runAt(time_point time, TimerCallbackFunc cb);
  TimerProxy runAfter(double timeS, TimerCallbackFunc cb);
  TimerProxy runEvery(double intervalS, TimerCallbackFunc cb);

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
  std::unique_ptr<TimerQueue> timer_queue_;
  // EventLoop has no ownership of channel!
  std::vector<Channel *> active_channels_;
};
} // namespace PD
