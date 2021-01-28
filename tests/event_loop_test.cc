#include "gtest/gtest.h"
#define GTEST_UNIT_TEST
#include "EventLoop.hh"
#include <future>

TEST(EventLoopTests, loopInThread) {
  auto threadfunc =  [](){
    PD::EventLoop loop;
    loop.run();
    return std::make_pair(std::this_thread::get_id(),loop.tid_);
  };
  auto res_future = std::async(std::launch::async,threadfunc);
  PD::EventLoop loop;
  loop.run();
  EXPECT_EQ(loop.tid_,std::this_thread::get_id());
  auto res = res_future.get();
  EXPECT_EQ(res.first,res.second);
  EXPECT_NE(res.first,loop.tid_);
}

TEST(EventLoopTests, loopInOtherThread) {
  PD::EventLoop loop;
  auto threadfunc =  [&loop](){
     spdlog::info("This thread pid is {} ",std::this_thread::get_id());
     ASSERT_DEATH(loop.run(),"");
  };
  std::thread t1(threadfunc);
  t1.detach();
}
