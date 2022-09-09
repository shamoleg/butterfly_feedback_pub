cmake_minimum_required(VERSION 3.2)
enable_testing()

add_executable(json_test json_test.cpp)
target_link_libraries(json_test LINK_PRIVATE cppmisc)
add_test(NAME json_test COMMAND json_test)

add_executable(thread_pool_test
    thread_pool_test.cpp
)
target_link_libraries(thread_pool_test LINK_PRIVATE cppmisc)
add_test(NAME thread_pool_test COMMAND thread_pool_test)

add_executable(task_queue_test
    task_queue_test.cpp
)
target_link_libraries(task_queue_test LINK_PRIVATE cppmisc)
add_test(NAME task_queue_test COMMAND task_queue_test)

add_executable(files_test
    files_test.cpp
)
target_link_libraries(files_test LINK_PRIVATE cppmisc)
add_test(NAME files_test COMMAND files_test)
