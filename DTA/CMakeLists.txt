cmake_minimum_required (VERSION 3.16)
project (DTA VERSION 1.0.0)

set (EXEC_FILE "dta")

set (HEADERS 

    headers/dta.hpp
    headers/test.hpp
)

set (SOURCES

    sources/test.cpp
    sources/dta.cpp
    sources/main.cpp
)

add_executable (${EXEC_FILE} ${SOURCES})

target_include_directories (${EXEC_FILE} PRIVATE headers)
target_link_libraries(${EXEC_FILE} PRIVATE QBDI)

if (DISASM)
target_compile_definitions(${EXEC_FILE} PRIVATE SHARED)
endif()

target_compile_features(${EXEC_FILE} PRIVATE cxx_std_20)


if(CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format")
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/*.[ch]pp
        ${PROJECT_SOURCE_DIR}/*.[ch]
    )

    add_custom_target(
        format
        COMMAND clang-format
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
    )
endif()

