//
// Created by BlurryLight on 2021/1/27.
//

#include "EventLoop.hh"
#include "Channel.hh"
#include "Poller.hh"
#include "fmt/ostream.h"
#include "spdlog/spdlog.h"
#include <poll.h>
using namespace PD;
constexpr int kPollTimeoutMS = 2'000;
static thread_local EventLoop *loopInThisThread = nullptr;
EventLoop::EventLoop()
    : tid_(std::this_thread::get_id()), looping_(false), quit_(false),
      poller_(std::make_unique<Poller>(this)) {
  spdlog::info("Loop is created in the tid {}\n", tid_);
  if (loopInThisThread) {
    spdlog::error("Another loop {} is in this thread {}! Abort!",
                  (uint64_t)loopInThisThread, tid_);
    std::abort();
  } else {
    loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {
  spdlog::info("Loop is destroyed in the tid {}\n", tid_);
  loopInThisThread = nullptr;
  looping_ = false;
}

void EventLoop::run() {
  assert(!looping_);
  assert_in_thread();
  looping_ = true;
  quit_ = false;
  while (!quit_) {
    poller_->poll(kPollTimeoutMS, active_channels_);
    for (const auto &ch : active_channels_) {
      ch->handle_events();
    }
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
void EventLoop::quit() { quit_ = true; }
