cmake_minimum_required(VERSION 3.10)
project(Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)


add_subdirectory(./dependencies/raylib)
add_executable(Test main.cpp common.c net.c)
target_link_libraries(Test PRIVATE raylib)
target_include_directories(Test PUBLIC ./)
if (WIN32)
  target_link_libraries(Test PUBLIC wsock32 ws2_32)
endif()