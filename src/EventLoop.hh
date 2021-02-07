//
// Created by BlurryLight on 2021/1/27.
//
#pragma once

#include "Callbacks.hh"
#include "TimerProxy.hh"
#include "utils.hh"
#include <atomic>
#include <memory>
#include <mutex>
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

  //=====safe to call from other thread
  void quit();
  TimerProxy runAt(time_point time, TimerCallbackFunc cb);
  TimerProxy runAfter(double timeS, TimerCallbackFunc cb);
  TimerProxy runEvery(double intervalS, TimerCallbackFunc cb);
  time_point poller_return_time() const { return poller_return_time_; }
  // best effort to run as soon as possible
  void run_in_loop(const Functor &func);
  void queue_in_loop(const Functor &func);
  //====safe to call from other thread

#ifndef GTEST_UNIT_TEST
  // expose all private members to GTEST
private:
#endif
  void wakeup();
  // abort when loop is not in that it was created
  void abort_not_in_thread() const;
  void handle_wakeup();
  void do_pending_functors();
  const std::thread::id tid_;
  int wakeup_fd_; // man eventfd
  std::atomic<bool> looping_;
  std::atomic<bool> quit_;
  std::atomic<bool> callingPendingFunctors_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  std::unique_ptr<Channel> wakeup_channel_;
  // EventLoop has no ownership of channel!
  std::vector<Channel *> active_channels_;
  std::mutex mutex_;
  std::vector<Functor> pending_funcs_;
  time_point poller_return_time_;
};
} // namespace PD
