//
// Created by BlurryLight on 2021/2/7.
//

#pragma once
#include "utils.hh"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
namespace PD {
class EventLoop;
class EventLoopThread : public Noncopyable {
public:
  EventLoopThread();
  ~EventLoopThread();
  std::weak_ptr<EventLoop> run_loop();

private:
  void thread_func();
  std::weak_ptr<EventLoop> loop_;
  bool exiting_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

} // namespace PD
