cmake_minimum_required(VERSION 3.5)

project(pscan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic")

add_executable(pscan_final pscan_final.cpp)
target_link_libraries(pscan_final fmt::fmt necopp)

add_executable(pscan1 pscan1.cpp)
target_link_libraries(pscan1 fmt::fmt necopp)

add_executable(pscan2 pscan2.cpp)
target_link_libraries(pscan2 fmt::fmt necopp)

add_executable(pscan3 pscan3.cpp)
target_link_libraries(pscan3 fmt::fmt necopp)

add_executable(pscan4 pscan4.cpp)
target_link_libraries(pscan4 fmt::fmt necopp)

add_executable(pscan5 pscan5.cpp)
target_link_libraries(pscan5 fmt::fmt necopp)
