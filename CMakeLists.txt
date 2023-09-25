cmake_minimum_required( VERSION 3.25 )

project( networkprotocoldsl CXX )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    networkprotocoldsl

    src/networkprotocoldsl/entrypoint.cpp
    src/networkprotocoldsl/entrypoint.hpp
    src/networkprotocoldsl/interpretedprogram.hpp
    src/networkprotocoldsl/interpretedprogram.cpp
    src/networkprotocoldsl/interpreter.hpp
    src/networkprotocoldsl/interpreter.cpp
    src/networkprotocoldsl/value.hpp
    src/networkprotocoldsl/value.cpp
    src/networkprotocoldsl/operation.hpp
    src/networkprotocoldsl/operation.cpp
    src/networkprotocoldsl/optree.hpp
    src/networkprotocoldsl/optree.cpp
)

target_include_directories(
    networkprotocoldsl
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

enable_testing()
add_subdirectory(tests)
