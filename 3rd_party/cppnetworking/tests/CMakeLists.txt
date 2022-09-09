cmake_minimum_required(VERSION 3.1)

enable_testing()
add_executable(udp_test test_udp.cpp)
target_link_libraries(udp_test PRIVATE networking)
add_test(NAME udp_test COMMAND udp_test)
