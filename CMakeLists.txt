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
    src/networkprotocoldsl/executionstackframe.cpp
    src/networkprotocoldsl/executionstackframe.hpp
    src/networkprotocoldsl/interpretedprogram.cpp
    src/networkprotocoldsl/interpretedprogram.hpp
    src/networkprotocoldsl/interpreter.cpp
    src/networkprotocoldsl/interpreter.hpp
    src/networkprotocoldsl/lexicalpad.cpp
    src/networkprotocoldsl/lexicalpad.hpp
    src/networkprotocoldsl/operation.cpp
    src/networkprotocoldsl/operation.hpp
    src/networkprotocoldsl/operation/add.cpp
    src/networkprotocoldsl/operation/add.hpp
    src/networkprotocoldsl/operation/dynamiclist.cpp
    src/networkprotocoldsl/operation/dynamiclist.hpp
    src/networkprotocoldsl/operation/eq.cpp
    src/networkprotocoldsl/operation/eq.hpp
    src/networkprotocoldsl/operation/functioncall.cpp
    src/networkprotocoldsl/operation/functioncall.hpp
    src/networkprotocoldsl/operation/if.cpp
    src/networkprotocoldsl/operation/if.hpp
    src/networkprotocoldsl/operation/int32literal.cpp
    src/networkprotocoldsl/operation/int32literal.hpp
    src/networkprotocoldsl/operation/lesserequal.cpp
    src/networkprotocoldsl/operation/lesserequal.hpp
    src/networkprotocoldsl/operation/lexicalpadget.cpp
    src/networkprotocoldsl/operation/lexicalpadget.hpp
    src/networkprotocoldsl/operation/lexicalpadinitialize.cpp
    src/networkprotocoldsl/operation/lexicalpadinitialize.hpp
    src/networkprotocoldsl/operation/lexicalpadinitializeglobal.cpp
    src/networkprotocoldsl/operation/lexicalpadinitializeglobal.hpp
    src/networkprotocoldsl/operation/lexicalpadset.cpp
    src/networkprotocoldsl/operation/lexicalpadset.hpp
    src/networkprotocoldsl/operation/multiply.cpp
    src/networkprotocoldsl/operation/multiply.hpp
    src/networkprotocoldsl/operation/opsequence.cpp
    src/networkprotocoldsl/operation/opsequence.hpp
    src/networkprotocoldsl/operation/readint32native.cpp
    src/networkprotocoldsl/operation/readint32native.hpp
    src/networkprotocoldsl/operation/staticcallable.cpp
    src/networkprotocoldsl/operation/staticcallable.hpp
    src/networkprotocoldsl/operation/subtract.cpp
    src/networkprotocoldsl/operation/subtract.hpp
    src/networkprotocoldsl/operation/unarycallback.cpp
    src/networkprotocoldsl/operation/unarycallback.hpp
    src/networkprotocoldsl/operation/writeint32native.cpp
    src/networkprotocoldsl/operation/writeint32native.hpp
    src/networkprotocoldsl/operationconcepts.cpp
    src/networkprotocoldsl/operationconcepts.hpp
    src/networkprotocoldsl/optree.cpp
    src/networkprotocoldsl/optree.hpp
    src/networkprotocoldsl/value.cpp
    src/networkprotocoldsl/value.hpp
)

target_include_directories(
    networkprotocoldsl
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_compile_options(
    networkprotocoldsl
    PUBLIC
    -Wall
#    -fconcepts-diagnostics-depth=10
)

enable_testing()
add_subdirectory(tests)
