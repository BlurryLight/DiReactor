cmake_minimum_required(VERSION 3.5)
project(UNITTEST LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

set(BASE_LIST
        ../src/EventLoop.cc
        ../src/Channel.cc
        ../src/Poller.cc
        )
set(TESTS_LIST
        # empty
        event_loop_test.cc
        poller_test.cc
        )

add_executable(runTests ${TESTS_LIST} ${BASE_LIST})
target_include_directories(runTests PRIVATE
        ../src/
        )

#add_executable(pollertest poller_test.cc ${BASE_LIST})
#target_include_directories(pollertest PRIVATE
#        ../src/
#        )
target_link_libraries(runTests PRIVATE GTest::GTest GTest::gtest_main Threads::Threads spdlog::spdlog)
#target_link_libraries(pollertest PRIVATE GTest::GTest GTest::gtest_main Threads::Threads spdlog::spdlog)
add_test(
        NAME runTests
        COMMAND runTests
)