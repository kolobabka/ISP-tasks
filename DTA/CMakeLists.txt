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

target_compile_features(${EXEC_FILE} PRIVATE cxx_std_20)
