cmake_minimum_required(VERSION 2.8)

enable_testing()

add_executable(test_servo_control test_servo_control.cpp)
target_link_libraries(test_servo_control "${CMAKE_THREAD_LIBS}" butterfly)

add_executable(test_servo_pos test_servo_pos.cpp)
target_link_libraries(test_servo_pos "${CMAKE_THREAD_LIBS}" butterfly)

add_executable(test_cam test_cam.cpp)
target_link_libraries(test_cam "${CMAKE_THREAD_LIBS}" butterfly)

add_executable(test_moving_average test_moving_average.cpp)
target_link_libraries(test_moving_average "${CMAKE_THREAD_LIBS}" butterfly)
add_test(name test_moving_average command test_moving_average)
