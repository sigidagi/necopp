cmake_minimum_required(VERSION 3.14)
project(llhttp_cpp_project LANGUAGES CXX C)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add nlohmann::json to the project
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2 # Use the latest stable version or specify a preferred version
)
FetchContent_MakeAvailable(json)

# Add the llhttp C library
add_subdirectory(llhttp)

# Include the directory with llhttp headers
include_directories(llhttp/include)

# Add your project sources
add_library(llhttpp INTERFACE)
target_include_directories(llhttpp INTERFACE include/)


# Link llhttp library to your C++ wrapper
target_link_libraries(llhttpp INTERFACE llhttp)

# Optional: Add an executable for testing
add_executable(example example/main.cpp)

# Link the wrapper to the executable
target_link_libraries(example PRIVATE llhttpp nlohmann_json::nlohmann_json)

