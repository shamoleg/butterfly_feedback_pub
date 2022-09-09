cmake_minimum_required(VERSION 2.8)

project(butterfly_feedback CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-march=native -mtune=native -Ofast")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(3rd_party/cppmisc)
add_subdirectory(3rd_party/cppnetworking)
find_package(Threads REQUIRED)

add_library(butterfly STATIC
	src/servo_iface.cpp
	src/servo_iface.h

	src/cam_iface.cpp
	src/cam_iface.h

	src/butterfly.cpp
	src/butterfly.h

	src/splines.cpp
	src/splines.h
)
target_link_libraries(butterfly "${CMAKE_THREAD_LIBS}" cppmisc networking)

add_executable(overturn_controller
	src/overturn_controller.h
 	src/overturn_controller.cpp
)
target_link_libraries(overturn_controller "${CMAKE_THREAD_LIBS}" butterfly)

add_executable(measurements src/measurements.cpp)
target_link_libraries(measurements "${CMAKE_THREAD_LIBS}" butterfly)

add_subdirectory(tests)
