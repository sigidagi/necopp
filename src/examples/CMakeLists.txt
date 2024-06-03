cmake_minimum_required(VERSION 3.5)

add_executable(coroutine coroutines.cpp)
target_link_libraries(coroutine PRIVATE fmt::fmt necopp)

add_executable(channel channels.cpp)
target_link_libraries(channel PRIVATE fmt::fmt necopp)

add_executable(generator generators.cpp)
target_link_libraries(generator PRIVATE fmt::fmt necopp)

add_executable(http_client  http_client.cpp)
target_link_libraries(http_client PRIVATE fmt::fmt necopp)

add_executable(http_server http_server.cpp)
target_link_libraries(http_server PRIVATE fmt::fmt necopp)

# Port scanner tutorial
add_subdirectory(pscan)

# HTTP parser tutorial
add_subdirectory(llhttp)
