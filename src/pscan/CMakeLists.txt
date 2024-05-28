cmake_minimum_required(VERSION 3.5)

project(pscan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic")

add_executable(pscan_coro pscan5.cpp)
target_link_libraries(pscan_coro fmt::fmt necopp)

add_executable(pscan_sing pscan2.cpp)
target_link_libraries(pscan_sing fmt::fmt necopp)
