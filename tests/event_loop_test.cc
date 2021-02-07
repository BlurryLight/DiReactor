#include "fmt/ostream.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <chrono>
#define GTEST_UNIT_TEST
#include "EventLoop.hh"
#include "EventLoopThread.hh"
#include <future>

TEST(EventLoopTests, loopInThread) {
  auto threadfunc = []() {
    PD::EventLoop loop;
    return std::make_pair(std::this_thread::get_id(), loop.tid_);
  };
  auto res_future = std::async(std::launch::async, threadfunc);
  PD::EventLoop loop;
  EXPECT_EQ(loop.tid_, std::this_thread::get_id());
  auto res = res_future.get();
  EXPECT_EQ(res.first, res.second);
  EXPECT_NE(res.first, loop.tid_);
}

TEST(EventLoopTests, loopInOtherThread) {
  PD::EventLoop loop;
  auto threadfunc = [&loop]() {
    spdlog::info("This thread pid is {} ", std::this_thread::get_id());
    loop.run();
  };
  ASSERT_DEATH({ std::thread t1(threadfunc); }, "");
}

TEST(EventLoopTests, MultiLoopsInOneThread) {
  PD::EventLoop loop;
  ASSERT_DEATH({ PD::EventLoop loop2; }, "");
  // will abort
}

TEST(EventLoopTests, TimerQueueRunEvery) {
  PD::EventLoop loop;
  int cnt = 0;
  auto func = [&]() {
    spdlog::info("cnt: {}", ++cnt);
    if (cnt >= 3) {
      loop.quit();
    }
  };
  loop.runEvery(1.0, func);
  loop.run();
  EXPECT_EQ(cnt, 3);
}

TEST(EventLoopTests, TimerQueueRunAfter) {
  PD::EventLoop loop;
  decltype(std::chrono::system_clock::now()) ts;
  auto func = [&]() {
    ts = std::chrono::system_clock::now();
    loop.quit();
  };
  auto start = std::chrono::system_clock::now();
  loop.runAfter(1.23456, func); // happen after 1.23456s
  loop.run();
  // happens > 1234 ms  < 1235 ms
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      1234);
  EXPECT_LE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      1235);
  spdlog::info("run after {} ms!",
               std::chrono::duration_cast<std::chrono::milliseconds>(ts - start)
                   .count());
}
TEST(EventLoopTests, TimerQueueRunAt) {
  PD::EventLoop loop;
  decltype(std::chrono::system_clock::now()) ts;
  auto func = [&]() {
    ts = std::chrono::system_clock::now();
    loop.quit();
  };
  auto start = std::chrono::system_clock::now();
  auto expected = start + std::chrono::milliseconds{789}; // happen after 789ms
  loop.runAt(expected, func);
  loop.run();
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      788);
  EXPECT_LE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      790);
  spdlog::info("run after {} ms!",
               std::chrono::duration_cast<std::chrono::milliseconds>(ts - start)
                   .count());
}
TEST(EventLoopTests, runInLoop) {
  PD::EventLoop loop;
  auto pid = std::this_thread::get_id();
  auto func = [&]() {
    EXPECT_EQ(pid, std::this_thread::get_id());
    loop.quit();
  };
  loop.runAfter(2.0, func);
  loop.run();
}

TEST(EventLoopTests, AddTimerInOtherThread) {
  PD::EventLoop loop;
  int flag = 0;
  auto func1 = [&]() {
    flag = 1;
    loop.quit();
  };
  auto func2 = [&loop, func1]() { loop.runAfter(2.0, func1); };
  (void)std::async(std::launch::async, func2);
  auto tp1 = std::chrono::system_clock::now();
  loop.run();
  auto tp2 = std::chrono::system_clock::now();
  EXPECT_EQ(flag, 1);
  EXPECT_FLOAT_EQ(
      2.0, std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1).count());
}

TEST(EventLoopTests, EventLoopThread) {
  std::weak_ptr<PD::EventLoop> loop;
  auto pid = std::this_thread::get_id();
  spdlog::info("Current thread is {}", pid);
  auto fun = [=]() { EXPECT_NE(std::this_thread::get_id(), pid); };
  {
    PD::EventLoopThread loop_thread;
    loop = loop_thread.run_loop();
    EXPECT_TRUE(!loop.expired());
    auto loop_ptr = loop.lock();
    loop_ptr->run_in_loop(fun);
  }
  EXPECT_TRUE(loop.expired());
}
