cmake_minimum_required(VERSION 3.5)

project(necopp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use e.g. GNU extension (like -std=gnu++11) for portability
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_options(-g -Wall -Wextra)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CPM)
CPMAddPackage("gh:sigidagi/neco@0.3.4")

# neco coroutine library wrapper
add_library(necopp
    src/necopp.cpp
)

target_include_directories(necopp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${neco_SOURCE_DIR}
)

target_link_libraries(necopp PRIVATE neco)

add_subdirectory(src/examples)
