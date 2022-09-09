cmake_minimum_required(VERSION 3.1)

project(networking CXX)
find_package(Threads)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(networking STATIC
    inc/networking/udp.h
    inc/networking/tcp.h
    src/udp.cpp
    src/tcp.cpp
)
target_link_libraries(networking PUBLIC cppmisc ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(networking PUBLIC ${PROJECT_SOURCE_DIR}/inc)

add_subdirectory(tests)
