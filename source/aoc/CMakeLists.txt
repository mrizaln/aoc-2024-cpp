add_executable(aoc ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(aoc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(aoc PRIVATE -Wall -Wextra -Wconversion)

target_link_libraries(
    aoc
    PRIVATE
        fmt::fmt
        CLI11::CLI11
        libassert::assert
        rapidhash::rapidhash
        magic_enum::magic_enum
)

set_target_properties(
    aoc
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# sanitizer
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(aoc PRIVATE -fsanitize=address,leak,undefined)
    target_link_options(aoc PRIVATE -fsanitize=address,leak,undefined)
endif()

add_custom_command(
    TARGET aoc
    COMMENT "Linking data directory to build directory"
    PRE_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/data/
        $<TARGET_FILE_DIR:aoc>/data
)
