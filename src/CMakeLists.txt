find_package(Threads REQUIRED)

set(SOURCE_LIST
    # empty
    )

add_executable(main main.cc ${SOURCE_LIST} ${SOURCE_HEADERS})
target_link_libraries(main Threads::Threads)
