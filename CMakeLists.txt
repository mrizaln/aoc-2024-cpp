cmake_minimum_required(VERSION 3.22)
project(aoc-cpp-template VERSION 0.0.0)

include(cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)
find_package(scn REQUIRED)
find_package(CLI11 REQUIRED)
find_package(nameof REQUIRED)

add_executable(main source/main.cpp)
target_include_directories(main PRIVATE source)
target_link_libraries(main PRIVATE fmt::fmt scn::scn CLI11::CLI11 nameof::nameof)
target_compile_options(main PRIVATE -Wall -Wextra -Wconversion -Wswitch-enum)

# sanitizer
target_compile_options(main PRIVATE -fsanitize=address,leak,undefined)
target_link_options(main PRIVATE -fsanitize=address,leak,undefined)
