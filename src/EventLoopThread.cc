//
// Created by BlurryLight on 2021/2/7.
//

#include "EventLoopThread.hh"
#include "EventLoop.hh"
#include "spdlog/spdlog.h"
#include <cassert>
using namespace PD;
PD::EventLoopThread::EventLoopThread() : loop_(), exiting_(false), thread_() {}
PD::EventLoopThread::~EventLoopThread() {
  assert(!exiting_);
  exiting_ = true;
  assert(!loop_.expired());
  loop_.lock()->quit();
  assert(thread_.joinable());
  thread_.join();
}
std::weak_ptr<EventLoop> EventLoopThread::run_loop() {
  assert(!thread_.joinable());
  thread_ = std::thread([this]() { thread_func(); });
  {
    std::unique_lock<std::mutex> lk(mutex_);
    while (loop_.expired()) {
      cv_.wait(lk);
    }
  }
  return loop_;
}
void EventLoopThread::thread_func() {
  spdlog::info("Thread func runs!");
  auto loop = std::make_shared<EventLoop>();
  {
    std::unique_lock<std::mutex> lk(mutex_);
    loop_ = loop;
    cv_.notify_one();
  }
  loop->run();
  spdlog::info("Thread func exit!");
  // loop end, must be in ~EventLoopThread
  // or maybe in quiting
  assert(exiting_);
}
