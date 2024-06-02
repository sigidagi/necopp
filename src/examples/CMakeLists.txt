cmake_minimum_required(VERSION 3.5)

add_executable(necocoro necocoro.cpp)
target_link_libraries(necocoro PRIVATE necopp)

add_executable(necochan necochan.cpp)
target_link_libraries(necochan PRIVATE necopp)

add_executable(necogen necogen.cpp)
target_link_libraries(necogen PRIVATE necopp)

add_executable(necoclient necoclient.cpp)
target_link_libraries(necoclient PRIVATE fmt::fmt necopp)

add_executable(necoserver necoserver.cpp)
target_link_libraries(necoserver PRIVATE fmt::fmt necopp)
