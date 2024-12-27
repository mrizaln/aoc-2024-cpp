cmake_minimum_required(VERSION 3.22)
project(aoc-cpp-template VERSION 0.0.0)

include(cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)
find_package(libassert REQUIRED)
find_package(rapidhash REQUIRED)
find_package(magic_enum REQUIRED)
find_package(SFML REQUIRED)

add_subdirectory(source/aoc)
add_subdirectory(source/vis)
