//
// Created by BlurryLight on 2021/1/27.
//
#pragma once

#include "utils.hh"
#include <atomic>
#include <thread>
namespace PD {
class EventLoop : public Noncopyable {
public:
  EventLoop();
  ~EventLoop();
  void run();
  [[nodiscard]] bool is_in_thread() const;
  void assert_in_thread() const;

#ifndef GTEST_UNIT_TEST
  // expose all private members to GTEST
private:
#endif
  // abort when loop is not in that it was creatd
  void abort_not_in_thread() const;
  const std::thread::id tid_;
  std::atomic<bool> looping_;
};
}; // namespace PD
