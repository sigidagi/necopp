cmake_minimum_required(VERSION 3.15)

project(necocoro)

# Don't use e.g. GNU extension (like -std=gnu++11) for portability
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Release only" FORCE)
add_compile_options(-Wall -Wextra)

# FetchContent for the `neco` library
include(FetchContent)

FetchContent_Declare(neco
    GIT_REPOSITORY https://github.com/tidwall/neco.git
    GIT_TAG main # Use the desired branch, tag, or commit
)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2 # Use the desired branch, tag, or commit
)
FetchContent_Declare(json 
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3 # Use the desired branch, tag, or commit
)

FetchContent_MakeAvailable(neco)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(json)

add_library(neco_static STATIC ${neco_SOURCE_DIR}/neco.c)
# Include the `neco` headers
target_include_directories(neco_static PUBLIC ${neco_SOURCE_DIR})

# neco coroutine library wrapper
add_library(necopp STATIC
    src/necopp.cpp
)

target_include_directories(necopp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${neco_SOURCE_DIR}
)

target_link_libraries(necopp PRIVATE neco_static)
target_compile_features(necopp PUBLIC cxx_std_17)

add_subdirectory(examples)
