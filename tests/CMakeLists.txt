cmake_minimum_required(VERSION 3.5)
project(UNITTEST LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(SOURCE_LIST
    # empty
    )

add_executable(runTests tests.cc ${SOURCE_LIST} ${SOURCE_HEADERS})
target_include_directories(runTests PRIVATE 
    ../src/
    ${GTEST_INCLUDE_DIRS}
    )
target_link_libraries(runTests GTest::GTest Threads::Threads)
add_test(
    NAME    runTests 
    COMMAND runTests 
)