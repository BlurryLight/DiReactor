//
// Created by BlurryLight on 2021/2/6.
//

#pragma once
#include "Callbacks.hh"
#include "utils.hh"
#include <chrono>
namespace PD {
using time_point = std::chrono::system_clock::time_point;
class Timer : public Noncopyable {
public:
  Timer(TimerCallbackFunc cb, time_point expiration, double intervalS = 0.0);
  void run() const;
  time_point expiration() const;
  bool repeat() const;
  void restart(time_point now);

private:
  const TimerCallbackFunc cb_;
  ;
  time_point expiration_;
  const double interval_ = 0.0;
  const bool repeat_ = false;
};

} // namespace PD
