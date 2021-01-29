find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

set(SOURCE_LIST
        utils.hh EventLoop.cc EventLoop.hh Channel.cc Channel.hh Poller.cc Poller.hh)

add_executable(main main.cc ${SOURCE_LIST} ${SOURCE_HEADERS})
target_link_libraries(main PRIVATE Threads::Threads spdlog::spdlog)
