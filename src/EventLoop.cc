//
// Created by BlurryLight on 2021/1/27.
//

#include "EventLoop.hh"
#include "Channel.hh"
#include "Poller.hh"
#include "Timer.hh"
#include "TimerQueue.hh"
#include "fmt/ostream.h"
#include "spdlog/spdlog.h"
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <utility>
using namespace PD;
constexpr int kPollTimeoutMS = 2'000;
static thread_local EventLoop *loopInThisThread = nullptr;
static int create_eventfd() {
  int eventfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (eventfd < 0) {
    spdlog::error("create_event fd failed!");
    std::abort();
  }
  return eventfd;
}
EventLoop::EventLoop()
    : tid_(std::this_thread::get_id()), looping_(false), quit_(false),
      poller_(std::make_unique<Poller>(this)),
      timer_queue_(std::make_unique<TimerQueue>(this)),
      callingPendingFunctors_(false), wakeup_fd_(create_eventfd()),
      wakeup_channel_(std::make_unique<Channel>(this, wakeup_fd_)) {
  spdlog::info("Loop is created in the tid {}\n", tid_);
  if (loopInThisThread) {
    spdlog::error("Another loop {} is in this thread {}! Abort!",
                  (uint64_t)loopInThisThread, tid_);
    std::abort();
  } else {
    loopInThisThread = this;
  }
  wakeup_channel_->set_read_callback([this] { handle_wakeup(); });
  wakeup_channel_->enable_reading();
}

EventLoop::~EventLoop() {
  spdlog::info("Loop is destroyed in the tid {}\n", tid_);
  loopInThisThread = nullptr;
  looping_ = false;
  ::close(wakeup_fd_);
}

void EventLoop::run() {
  assert(!looping_);
  assert_in_thread();
  looping_ = true;
  quit_ = false;
  while (!quit_) {
    poller_return_time_ = poller_->poll(kPollTimeoutMS, active_channels_);
    for (const auto &ch : active_channels_) {
      ch->handle_events();
    }
    do_pending_functors();
  }
  spdlog::info("loop {} stop looping!", (uint64_t)this);
  looping_ = false;
}
bool EventLoop::is_in_thread() const {
  return tid_ == std::this_thread::get_id();
}
void EventLoop::abort_not_in_thread() const {
  spdlog::error("EventLoop abort_not_in_thread:"
                "Current tid {} ,the tid loop "
                "was created: {} \n",
                std::this_thread::get_id(), tid_);
  std::abort();
}
void EventLoop::assert_in_thread() const {
  if (!is_in_thread())
    abort_not_in_thread();
}
void EventLoop::update_channel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assert_in_thread();
  poller_->update_channel(channel);
}
void EventLoop::quit() {
  quit_ = true;

  if (!is_in_thread() || callingPendingFunctors_)
    wakeup();
}
TimerProxy EventLoop::runAt(time_point time, TimerCallbackFunc cb) {
  return timer_queue_->addTimer(cb, time);
}
TimerProxy EventLoop::runAfter(double timeS, TimerCallbackFunc cb) {
  time_point tm = std::chrono::system_clock::now() +
                  std::chrono::milliseconds(static_cast<int>(timeS * 1000));
  return runAt(tm, std::move(cb));
}
TimerProxy EventLoop::runEvery(double intervalS, TimerCallbackFunc cb) {
  time_point tm = std::chrono::system_clock::now() +
                  std::chrono::milliseconds(static_cast<int>(intervalS * 1000));
  return timer_queue_->addTimer(cb, tm, intervalS);
}
void EventLoop::wakeup() {
  uint64_t tmp = 1;
  ssize_t n = ::write(wakeup_fd_, &tmp, sizeof tmp);
  if (n != sizeof tmp) {
    spdlog::error("wakeup write {} instead of {}", n, sizeof tmp);
  }
}
void EventLoop::handle_wakeup() {
  uint64_t tmp;
  ssize_t n = ::read(wakeup_fd_, &tmp, sizeof tmp);
  if (n != sizeof tmp) {
    spdlog::error("handle wakeup read {} instead of {}", n, sizeof tmp);
  }
}
void EventLoop::do_pending_functors() {
  callingPendingFunctors_ = true;
  std::vector<Functor> pending_funcs;
  {
    std::lock_guard<std::mutex> lk(mutex_);
    pending_funcs.swap(pending_funcs_);
  }
  for (const auto &func : pending_funcs) {
    func();
  }
  callingPendingFunctors_ = false;
}
void EventLoop::run_in_loop(const Functor &func) {
  if (is_in_thread()) {
    func();
  } else {
    queue_in_loop(func);
  }
}
void EventLoop::queue_in_loop(const Functor &func) {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    pending_funcs_.push_back(func);
  }
  if (!is_in_thread() || callingPendingFunctors_) {
    //为什么callingPendingFunctors的时候要唤醒？
    //因为calling的时间在while循环的末尾，如果此时不write eventfd
    //下一次poll的时候不会发现有新的functor在排队，可能一直阻塞在poll上
    //只有在处理IO回调的时候不用wakeup，因为此时代码运行在poll后，callpending之前，
    //运行到doPendingFunctors的时候会调用入队的函数
    wakeup();
  }
}
void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assert_in_thread();
  poller_->remove_channel(channel);
}
