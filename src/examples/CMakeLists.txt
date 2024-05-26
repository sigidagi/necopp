cmake_minimum_required(VERSION 3.5)

add_executable(necocoro
    ${CMAKE_CURRENT_SOURCE_DIR}/necocoro.cpp
)

target_include_directories(necocoro PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(necocoro PRIVATE
    necopp
)

add_executable(necochan
    necochan.cpp
)

target_include_directories(necochan PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(necochan PRIVATE
    necopp
)

add_executable(necogen
    necogen.cpp
)

target_include_directories(necogen PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(necogen PRIVATE
    necopp
)

