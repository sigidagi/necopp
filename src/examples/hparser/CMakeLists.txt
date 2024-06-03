cmake_minimum_required(VERSION 3.5)

add_executable(simple simple.c)
target_link_libraries(simple PRIVATE llhttp::llhttp)

add_executable(simplepp simplepp.cpp)
target_link_libraries(simplepp PRIVATE fmt::fmt hparser)
