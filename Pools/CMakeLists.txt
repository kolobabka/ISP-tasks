cmake_minimum_required (VERSION 3.16)
project (Pools VERSION 1.0.0)

set (EXEC_FILE "pools")

set (HEADERS

    headers/pools.hpp
)

set (SOURCES

    sources/pools.cpp
    sources/main.cpp
)

add_executable (${EXEC_FILE} ${SOURCES})
target_include_directories(${EXEC_FILE} PRIVATE headers)


set (COMPILE_SETTINGS 

    -O2
    -Wall
    -Wextra
    -Wpedantic
    # -Werror
)

target_compile_options(${EXEC_FILE} PRIVATE ${COMPILE_SETTINGS})
