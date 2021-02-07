//
// Created by BlurryLight on 2021/2/6.
//

#pragma once
#include "Callbacks.hh"
#include "Channel.hh"
#include "utils.hh"
#include <chrono>
#include <memory>
#include <set>
#include <utility>
#include <vector>
namespace PD {
// fwd
class Timer;
class EventLoop;
class Channel;
class TimerProxy;

using time_point = std::chrono::system_clock::time_point;
class TimerQueue : public Noncopyable {

public:
  TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerProxy addTimer(const TimerCallbackFunc &cb, time_point expiration,
                      double intervalS = 0.0);

private:
  void addTimerInLoop(Timer *timer);
  using Entry = std::pair<time_point, std::unique_ptr<Timer>>;
  using TimerList = std::set<Entry>;

  // read when timerfd pollin
  void handle_timerfd();
  std::vector<Entry> getExpired(time_point now);
  void reset(std::vector<Entry> &expired, time_point now);
  bool insert(Timer *timer);
  bool insert(std::unique_ptr<Timer> timer);

  EventLoop *loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  TimerList timers_;
};

} // namespace PD
