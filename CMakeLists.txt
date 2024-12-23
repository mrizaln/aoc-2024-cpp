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

add_executable(main source/main.cpp)
target_include_directories(main PRIVATE source)
target_link_libraries(main PRIVATE fmt::fmt CLI11::CLI11 libassert::assert rapidhash::rapidhash)
target_compile_options(main PRIVATE -Wall -Wextra -Wconversion)

# sanitizer
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(main PRIVATE -fsanitize=address,leak,undefined)
  target_link_options(main PRIVATE -fsanitize=address,leak,undefined)
endif()

add_custom_command(
  TARGET main
  COMMENT "Linking data directory to build directory"
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/data/
          $<TARGET_FILE_DIR:main>/data)
