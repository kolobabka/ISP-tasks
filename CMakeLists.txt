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


target_compile_features(${EXEC_FILE} PRIVATE cxx_std_17)
