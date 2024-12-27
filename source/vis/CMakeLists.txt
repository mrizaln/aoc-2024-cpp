function(create_executable name)
    add_executable(vis-${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.cpp)
    target_include_directories(vis-${name} PRIVATE ${CMAKE_SOURCE_DIR}/source/aoc)
    target_compile_options(vis-${name} PRIVATE -Wall -Wextra -Wconversion)
    target_link_libraries(
        vis-${name}
        PRIVATE fmt::fmt libassert::assert rapidhash::rapidhash sfml::sfml
    )
    set_target_properties(
        vis-${name}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

create_executable(day15)
