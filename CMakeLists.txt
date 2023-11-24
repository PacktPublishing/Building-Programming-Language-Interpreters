cmake_minimum_required( VERSION 3.25 )

project( networkprotocoldsl CXX )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    networkprotocoldsl

    src/networkprotocoldsl/continuation.cpp
    src/networkprotocoldsl/continuation.hpp
    src/networkprotocoldsl/entrypoint.cpp
    src/networkprotocoldsl/entrypoint.hpp
    src/networkprotocoldsl/executionstackframe.hpp
    src/networkprotocoldsl/executionstackframe.cpp
    src/networkprotocoldsl/interpretedprogram.hpp
    src/networkprotocoldsl/interpretedprogram.cpp
    src/networkprotocoldsl/interpreter.hpp
    src/networkprotocoldsl/interpreter.cpp
    src/networkprotocoldsl/value.hpp
    src/networkprotocoldsl/value.cpp
    src/networkprotocoldsl/operation/int32literal.hpp
    src/networkprotocoldsl/operation/int32literal.cpp
    src/networkprotocoldsl/operation/add.hpp
    src/networkprotocoldsl/operation/add.cpp
    src/networkprotocoldsl/operation/eq.hpp
    src/networkprotocoldsl/operation/eq.cpp
    src/networkprotocoldsl/operation/if.hpp
    src/networkprotocoldsl/operation/if.cpp
    src/networkprotocoldsl/operation/unarycallback.hpp
    src/networkprotocoldsl/operation/unarycallback.cpp
    src/networkprotocoldsl/operation/opsequence.hpp
    src/networkprotocoldsl/operation/opsequence.cpp
    src/networkprotocoldsl/operation/readint32native.hpp
    src/networkprotocoldsl/operation/readint32native.cpp
    src/networkprotocoldsl/operation/staticcallable.hpp
    src/networkprotocoldsl/operation/staticcallable.cpp
    src/networkprotocoldsl/operation/writeint32native.hpp
    src/networkprotocoldsl/operation/writeint32native.cpp
    src/networkprotocoldsl/operation.hpp
    src/networkprotocoldsl/operation.cpp
    src/networkprotocoldsl/operationconcepts.hpp
    src/networkprotocoldsl/operationconcepts.cpp
    src/networkprotocoldsl/optree.hpp
    src/networkprotocoldsl/optree.cpp
)

target_include_directories(
    networkprotocoldsl
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_compile_options(
    networkprotocoldsl
    PUBLIC
    -fconcepts-diagnostics-depth=10
)

enable_testing()
add_subdirectory(tests)
