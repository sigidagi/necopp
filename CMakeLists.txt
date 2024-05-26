cmake_minimum_required(VERSION 3.5)

project(necocoro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use e.g. GNU extension (like -std=gnu++11) for portability
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_options(-g -Wall -Wextra -Wpedantic)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CPM)
CPMAddPackage("gh:sigidagi/neco@0.3.4")
CPMAddPackage("gh:fmtlib/fmt#10.2.1")

add_library(necopp
    src/necopp.cpp
)

target_link_libraries(necopp PRIVATE neco)

target_include_directories(necopp PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${neco_SOURCE_DIR}
)

add_subdirectory(src/examples)
add_subdirectory(src/pscan)
