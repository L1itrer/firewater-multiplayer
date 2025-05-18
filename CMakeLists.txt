cmake_minimum_required(VERSION 3.10)
project(FireWater)

set(CMAKE_C_STANDARD 11)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

add_subdirectory(src/client/dependencies/raylib)
add_library(common src/common/common.c src/common/common.h)
target_include_directories(common PUBLIC    src/common)

add_executable(FireWater src/client/main.c src/client/net.c)
target_link_libraries(FireWater PRIVATE raylib)
target_link_libraries(FireWater PUBLIC common)
target_include_directories(FireWater PUBLIC src/common/)


add_executable(Server src/server/main.c)
target_link_libraries(Server common)
if (WIN32)
	target_link_libraries(Server wsock32 ws2_32)
  target_link_libraries(FireWater PUBLIC wsock32 ws2_32)
endif()
target_include_directories(Server PUBLIC src/common/)
