//
// Created by BlurryLight on 2021/2/6.
//

#include <cassert>

#include "Timer.hh"
using namespace PD;
Timer::Timer(TimerCallbackFunc cb, time_point expiration, double intervalS)
    : cb_(std::move(cb)), expiration_(expiration), interval_(intervalS),
      repeat_(intervalS > 0) {}
void Timer::run() const { cb_(); }
time_point Timer::expiration() const { return expiration_; }
bool Timer::repeat() const { return repeat_; }
void Timer::restart(time_point now) {
  assert(repeat());
  expiration_ =
      now + std::chrono::milliseconds{static_cast<int>(interval_ * 1000)};
}
