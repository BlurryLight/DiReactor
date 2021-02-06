//
// Created by BlurryLight on 2021/1/29.
//
#include "fmt/ostream.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#define GTEST_UNIT_TEST
#include "Channel.hh"
#include "EventLoop.hh"
#include "Poller.hh"
#include <iostream>
#include <sys/timerfd.h>

using namespace PD;
TEST(PollerTest, loopInThread) {
  EventLoop g_loop;
  int val = 0;
  auto timeout = [&]() {
    spdlog::info("timeout!");
    val = 1;
    g_loop.quit();
  };
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
  Channel channel(&g_loop, timerfd);
  channel.set_read_callback(timeout);
  channel.enable_reading();

  struct itimerspec interval;
  bzero(&interval, sizeof interval);
  interval.it_value.tv_sec = 1; // 1s to expire
  if (::timerfd_settime(timerfd, 0, &interval, nullptr) < 0)
    perror("error");
  g_loop.run();
  EXPECT_EQ(val, 1);
  EXPECT_EQ(g_loop.quit_, true);
  EXPECT_EQ(g_loop.looping_, false);
  ::close(timerfd);
}
