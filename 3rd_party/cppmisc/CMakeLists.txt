cmake_minimum_required(VERSION 3.2)

project(cppmisc CXX)
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(cppmisc STATIC
    src/json.cpp
    src/signals.cpp
    src/traces.cpp
    src/strings.cpp
    src/files.cpp
    inc/cppmisc/formatting.h
    inc/cppmisc/throws.h
    inc/cppmisc/timing.h
    inc/cppmisc/json.h
    inc/cppmisc/argparse.h
    inc/cppmisc/traces.h
    inc/cppmisc/signals.h
    inc/cppmisc/strings.h
    inc/cppmisc/files.h
    inc/cppmisc/threads.h
)
target_link_libraries(cppmisc PUBLIC jsoncpp ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(cppmisc PUBLIC ${PROJECT_SOURCE_DIR}/inc)

add_subdirectory(tests)
