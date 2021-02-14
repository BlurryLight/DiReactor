find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

set(SOURCE_LIST
        utils.hh EventLoop.cc EventLoop.hh Channel.cc Channel.hh Poller.cc Poller.hh Callbacks.hh Timer.cc Timer.hh TimerQueue.cc TimerQueue.hh TimerProxy.hh EventLoopThread.cc EventLoopThread.hh Acceptor.cc Acceptor.hh SocketUtils.hh SocketUtils.cc Socket.cc Socket.hh InetAddress.cc InetAddress.hh TcpServer.cc TcpServer.hh TcpConnection.cc TcpConnection.hh)

add_executable(main main.cc ${SOURCE_LIST} ${SOURCE_HEADERS})
target_link_libraries(main PRIVATE Threads::Threads spdlog::spdlog)
