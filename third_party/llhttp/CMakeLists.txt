cmake_minimum_required(VERSION 3.5)

project(llhttp)

add_compile_options(-g -Wall -Wextra -Wno-unused-parameter)

add_library(llhttp STATIC
    llhttp.c
    api.c
    http.c
)

add_library(llhttp::llhttp ALIAS llhttp)

target_include_directories(llhttp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET llhttp PROPERTY C_STANDARD 99)
