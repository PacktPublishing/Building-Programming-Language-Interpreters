cmake_minimum_required( VERSION 3.25 )

project( networkprotocoldsl CXX )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Enable ThreadSanitizer for Debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
 message(STATUS "Enabling ThreadSanitizer")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
 set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
endif()

include(FetchContent)
FetchContent_Declare(
  lexertl
  GIT_REPOSITORY https://github.com/BenHanson/lexertl17.git
  GIT_TAG        34fa04e5f2dfec551bbe92eae96c882a35238096 # 2024.02.17
)
FetchContent_MakeAvailable(lexertl)

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
    src/networkprotocoldsl/interpretercollection.cpp
    src/networkprotocoldsl/interpretercollection.hpp
    src/networkprotocoldsl/interpretercontext.cpp
    src/networkprotocoldsl/interpretercontext.hpp
    src/networkprotocoldsl/interpretercollectionmanager.cpp
    src/networkprotocoldsl/interpretercollectionmanager.hpp
    src/networkprotocoldsl/interpreterrunner.cpp
    src/networkprotocoldsl/interpreterrunner.hpp
    src/networkprotocoldsl/lexer/token.hpp
    src/networkprotocoldsl/lexer/token.cpp
    src/networkprotocoldsl/lexer/tokenize.hpp
    src/networkprotocoldsl/lexer/tokenize.cpp
    src/networkprotocoldsl/lexer/token/identifier.hpp
    src/networkprotocoldsl/lexer/token/identifier.cpp
    src/networkprotocoldsl/lexer/token/keyword.hpp
    src/networkprotocoldsl/lexer/token/keyword.cpp
    src/networkprotocoldsl/lexer/token/literal.hpp
    src/networkprotocoldsl/lexer/token/literal.cpp
    src/networkprotocoldsl/lexer/token/punctuation.hpp
    src/networkprotocoldsl/lexer/token/punctuation.cpp
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
    src/networkprotocoldsl/operation/functioncallforeach.cpp
    src/networkprotocoldsl/operation/functioncallforeach.hpp
    src/networkprotocoldsl/operation/generatelist.cpp
    src/networkprotocoldsl/operation/generatelist.hpp
    src/networkprotocoldsl/operation/if.cpp
    src/networkprotocoldsl/operation/if.hpp
    src/networkprotocoldsl/operation/int32literal.cpp
    src/networkprotocoldsl/operation/int32literal.hpp
    src/networkprotocoldsl/operation/inttoascii.cpp
    src/networkprotocoldsl/operation/inttoascii.hpp
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
    src/networkprotocoldsl/operation/readintfromascii.cpp
    src/networkprotocoldsl/operation/readintfromascii.hpp
    src/networkprotocoldsl/operation/readoctetsuntilterminator.cpp
    src/networkprotocoldsl/operation/readoctetsuntilterminator.hpp
    src/networkprotocoldsl/operation/readstaticoctets.cpp
    src/networkprotocoldsl/operation/readstaticoctets.hpp
    src/networkprotocoldsl/operation/staticcallable.cpp
    src/networkprotocoldsl/operation/staticcallable.hpp
    src/networkprotocoldsl/operation/subtract.cpp
    src/networkprotocoldsl/operation/subtract.hpp
    src/networkprotocoldsl/operation/terminatelistifreadahead.cpp
    src/networkprotocoldsl/operation/terminatelistifreadahead.hpp
    src/networkprotocoldsl/operation/unarycallback.cpp
    src/networkprotocoldsl/operation/unarycallback.hpp
    src/networkprotocoldsl/operation/writeint32native.cpp
    src/networkprotocoldsl/operation/writeint32native.hpp
    src/networkprotocoldsl/operation/writeoctets.cpp
    src/networkprotocoldsl/operation/writeoctets.hpp
    src/networkprotocoldsl/operation/writestaticoctets.cpp
    src/networkprotocoldsl/operation/writestaticoctets.hpp
    src/networkprotocoldsl/parser/booleanliteral.cpp
    src/networkprotocoldsl/parser/booleanliteral.hpp
    src/networkprotocoldsl/parser/identifierreference.cpp
    src/networkprotocoldsl/parser/identifierreference.hpp
    src/networkprotocoldsl/parser/integerliteral.cpp
    src/networkprotocoldsl/parser/integerliteral.hpp
    src/networkprotocoldsl/parser/message.cpp
    src/networkprotocoldsl/parser/messagedata.cpp
    src/networkprotocoldsl/parser/messagedata.hpp
    src/networkprotocoldsl/parser/messageforloop.cpp
    src/networkprotocoldsl/parser/messageforloop.hpp
    src/networkprotocoldsl/parser/message.hpp
    src/networkprotocoldsl/parser/messagepart.cpp
    src/networkprotocoldsl/parser/messagepart.hpp
    src/networkprotocoldsl/parser/messagesequence.cpp
    src/networkprotocoldsl/parser/messagesequence.hpp
    src/networkprotocoldsl/parser/protocoldescription.cpp
    src/networkprotocoldsl/parser/protocoldescription.hpp
    src/networkprotocoldsl/parser/stringliteral.cpp
    src/networkprotocoldsl/parser/stringliteral.hpp
    src/networkprotocoldsl/parser/terminator.cpp
    src/networkprotocoldsl/parser/terminator.hpp
    src/networkprotocoldsl/parser/tokenpart.cpp
    src/networkprotocoldsl/parser/tokenpart.hpp
    src/networkprotocoldsl/parser/tokensequence.cpp
    src/networkprotocoldsl/parser/tokensequence.hpp
    src/networkprotocoldsl/parser/type.cpp
    src/networkprotocoldsl/parser/type.hpp
    src/networkprotocoldsl/parser/typeparametermap.cpp
    src/networkprotocoldsl/parser/typeparametermap.hpp
    src/networkprotocoldsl/parser/typeparametervalue.cpp
    src/networkprotocoldsl/parser/typeparametervalue.hpp
    src/networkprotocoldsl/support/mutexlockqueue.cpp
    src/networkprotocoldsl/support/mutexlockqueue.hpp
    src/networkprotocoldsl/support/notificationsignal.cpp
    src/networkprotocoldsl/support/notificationsignal.hpp
    src/networkprotocoldsl/support/transactionalcontainer.cpp
    src/networkprotocoldsl/support/transactionalcontainer.hpp
    src/networkprotocoldsl/operationconcepts.cpp
    src/networkprotocoldsl/operationconcepts.hpp
    src/networkprotocoldsl/optree.cpp
    src/networkprotocoldsl/optree.hpp
    src/networkprotocoldsl/value.cpp
    src/networkprotocoldsl/value.hpp
)

target_link_libraries( networkprotocoldsl PUBLIC lexertl)

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
