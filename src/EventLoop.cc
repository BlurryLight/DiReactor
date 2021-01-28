//
// Created by BlurryLight on 2021/1/27.
//

#include "EventLoop.hh"
#include <poll.h>
using namespace PD;

static thread_local EventLoop *loopInThisThread = nullptr;
EventLoop::EventLoop() : tid_(std::this_thread::get_id()), looping_(false) {
  spdlog::info("Loop is created in the tid {}\n", tid_);
  if (loopInThisThread) {
    spdlog::error("Another loop {} is in this thread {}!",
                  (uint64_t)loopInThisThread, tid_);
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
  ::poll(NULL, 0, 5 * 1000);
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
