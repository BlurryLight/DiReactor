//
// Created by BlurryLight on 2021/2/6.
//

#pragma once
namespace PD {
class Timer;
class TimerProxy {
public:
  explicit TimerProxy(Timer *timer) : timer_(timer) {}

private:
  const Timer *timer_;
};

} // namespace PD
