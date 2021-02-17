//
// Created by BlurryLight on 2021/1/28.
//

#include "Channel.hh"
#include "EventLoop.hh"
#include "poll.h"
#include "spdlog/spdlog.h"

constexpr int kNoneEvent = 0;
constexpr int kReadEvent = POLLIN | POLLPRI | POLLRDHUP; // man 2 poll
constexpr int kWriteEvent = POLLOUT;
constexpr int kErrorEvent = POLLNVAL | POLLERR;
using namespace PD;
int Channel::enable_reading() {
  int oldevents = events_;
  events_ |= kReadEvent;
  update();
  return oldevents;
}
void Channel::set_read_callback(const Channel::EventCallbackFunc &cb) {
  readCb_ = cb;
}
void Channel::set_write_callback(const Channel::EventCallbackFunc &cb) {
  writeCb_ = cb;
}
void Channel::set_error_callback(const Channel::EventCallbackFunc &cb) {
  errCb_ = cb;
}
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1),
      event_handling_(false) {}
const EventLoop *Channel::ownerLoop() const { return loop_; }
void Channel::handle_events() {
  event_handling_ = true;
  if (revents_ & kErrorEvent) {
    if (revents_ & POLLNVAL) {
      spdlog::warn("Channel POLLNVAL, fd is {} ", fd_);
    }
    if (errCb_)
      errCb_();
  }

  if (revents_ & POLLHUP &&
      !(revents_ & POLLIN)) { // close and still has data to read
    spdlog::warn("Channel POLLHUP, fd is {} ", fd_);
    if (closeCb_)
      closeCb_();
  }

  if (revents_ & kReadEvent) {
    if (readCb_)
      readCb_();
  }

  if (revents_ & kWriteEvent) {
    if (writeCb_)
      writeCb_();
  }
  event_handling_ = false;
}
bool Channel::is_none_event() const { return events_ == kNoneEvent; }
void Channel::update() { loop_->update_channel(this); }
int Channel::fd() const { return fd_; }
int Channel::events() const { return events_; }
void Channel::set_revents(int revents) { revents_ = revents; }
int Channel::index() const { return index_; }
void Channel::set_index(int index) { index_ = index; }
Channel::~Channel() { assert(!event_handling_); }
void Channel::disable_all() {
  events_ = kNoneEvent;
  update();
}
int Channel::enable_writing() {
  int oldevents = events_;
  events_ |= kWriteEvent;
  update();
  return oldevents;
}

int Channel::disable_writing() {
  int oldevents = events_;
  events_ &= ~kWriteEvent;
  update();
  return oldevents;
}
void Channel::set_close_callback(const EventCallbackFunc &cb) { closeCb_ = cb; }
