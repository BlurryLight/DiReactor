//
// Created by BlurryLight on 2021/1/28.
//

#pragma once
#include "utils.hh"
#include <functional>

namespace PD {
class EventLoop;
// Channel is to handle events on fd
// Channel has none ownership of fd
// fd should be closed manually
class Channel : public Noncopyable {
public:
  using EventCallbackFunc = std::function<void()>;
  Channel(EventLoop *loop, int fd);
  int fd() const;
  int events() const;
  // poller will set this to deliver event
  void set_revents(int revents);
  [[nodiscard]] int index() const;
  // poller will set it
  void set_index(int index);
  int enable_reading();
  //  int enable_writing();
  void set_read_callback(const EventCallbackFunc &cb);
  void set_write_callback(const EventCallbackFunc &cb);
  void set_error_callback(const EventCallbackFunc &cb);
  [[nodiscard]] bool is_none_event() const;
  [[nodiscard]] const EventLoop *ownerLoop() const;
  void handle_events();

#ifndef GTEST_UNIT_TEST
private:
#endif
  void update();
  const int fd_;
  EventLoop *loop_;
  int events_; // man page poll
  int revents_;
  int index_; // index of Channel in Poller's vector

  EventCallbackFunc readCb_;
  EventCallbackFunc writeCb_;
  EventCallbackFunc errCb_;
};
}; // namespace PD
