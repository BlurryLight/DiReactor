#include "fmt/ostream.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <chrono>
#define GTEST_UNIT_TEST
#include "Acceptor.hh"
#include "EventLoop.hh"
#include "EventLoopThread.hh"
#include "InetAddress.hh"
#include "SocketUtils.hh"
#include "TcpConnection.hh"
#include "TcpServer.hh"
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
      1234 - 25);
  EXPECT_LE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      1234 + 25);
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
  // jitter by 25ms
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      789 - 25);
  EXPECT_LE(
      std::chrono::duration_cast<std::chrono::milliseconds>(ts - start).count(),
      789 + 25);
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
TEST(AcceptorTest, EventLoopThread) {
  int gval = 0;
  std::mutex lock;
  std::condition_variable cv;
  PD::EventLoopThread loop_thread;
  auto loop = loop_thread.run_loop();
  PD::InetAddress listenAddr(10086);
  PD::Acceptor acceptor(loop.lock().get(), listenAddr);
  auto conn_callback = [&lock, &cv, &gval](int fd,
                                           const PD::InetAddress &peer) {
    spdlog::info("new connection from {}", peer.to_host_port_str());
    {
      std::unique_lock<std::mutex> hold(lock);
      gval = 1;
      cv.notify_one();
    }
    PD::close(fd);
  };
  acceptor.set_new_conn_callback(conn_callback);
  loop.lock()->run_in_loop([&acceptor]() { acceptor.listen(); });
  while (!acceptor.isListening()) // data race here
    sleep(1);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  auto server_addr = listenAddr.get_sockaddr_struct();
  connect(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr);
  PD::close(sockfd);
  // data race here, so we sleep 1s
  sleep(1);
  {
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> hold(lock);
    cv.wait_for(hold, 1s, [gval]() { return gval == 0; });
  }
  EXPECT_EQ(gval, 1);
}

TEST(AcceptorTest, DaytimeServer) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y");
  auto str = oss.str();
  spdlog::info("Time is {} ", str);

  PD::EventLoopThread loop_thread;
  auto loop = loop_thread.run_loop();
  PD::InetAddress listenAddr(10086);
  PD::Acceptor acceptor(loop.lock().get(), listenAddr);
  auto conn_callback = [str](int fd, const PD::InetAddress &peer) {
    spdlog::info("new connection from {}", peer.to_host_port_str());
    ::send(fd, str.c_str(), str.size(), 0);
    ::shutdown(fd, SHUT_WR);
  };
  acceptor.set_new_conn_callback(conn_callback);
  loop.lock()->run_in_loop([&acceptor]() { acceptor.listen(); });
  while (!acceptor.isListening()) // data race here
    sleep(1);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  auto server_addr = listenAddr.get_sockaddr_struct();
  connect(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr);
  char buf[64];
  int n = recv(sockfd, buf, sizeof buf, 0);
  buf[n] = '\0';
  PD::close(sockfd);
  EXPECT_EQ(n, str.size());
  EXPECT_STREQ(str.c_str(), buf);
}

TEST(TcpServerTest, discard) {
  int msg_flag = 0;
  PD::EventLoopThread loop_thread;
  auto loop = loop_thread.run_loop();
  PD::InetAddress listenAddr(10086);
  PD::TcpServer server(loop.lock().get(), listenAddr);
  auto conn_callback = [](const PD::TcpConnectionPtr &conn) {
    spdlog::info("On connection() : new connection {} from {}", conn->name(),
                 conn->peerAddress().to_host_port_str());
  };

  auto msg_callback = [&msg_flag](const PD::TcpConnectionPtr &conn,
                                  const char *buf, ssize_t len) {
    spdlog::info("On message: received {} bytes from {} ", len,
                 conn->peerAddress().to_host_port_str());
    msg_flag = 1;
    EXPECT_EQ(len, 6);
    EXPECT_STREQ(buf, "Hello");
  };
  server.setConnectionCallback(conn_callback);
  server.setMessageCallback(msg_callback);
  server.start();
  sleep(1);

  // client
  {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    auto server_addr = listenAddr.get_sockaddr_struct();
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr);
    std::string str("Hello\0", 6);
    EXPECT_EQ(str.size(), 6);
    int n = send(sockfd, str.data(), str.size(), 0);
    sleep(2);
    EXPECT_EQ(str.size(), n);
    //  loop.lock()->quit();
    EXPECT_EQ(msg_flag, 1);
    PD::close(sockfd);
  }
  sleep(2);
  // will abort here
  // Tcpserver析构的时候带着conn析构了，但是channel仍然注册在Eventloop里面
}
