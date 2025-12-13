cmake_minimum_required( VERSION 3.25 )

project( networkprotocoldsl CXX )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect libuv for the UV adapter target only.
find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARIES NAMES uv libuv)
if(NOT LIBUV_INCLUDE_DIR)
  message(FATAL_ERROR "Libuv include directory not found")
endif()
if(NOT LIBUV_LIBRARIES)
  message(FATAL_ERROR "Libuv libraries not found")
endif()

# # Enable sanitizers for Debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
 message(STATUS "Enabling Sanitizers")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
 set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
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
    src/networkprotocoldsl/operation/dictionaryget.cpp
    src/networkprotocoldsl/operation/dictionaryinitialize.cpp
    src/networkprotocoldsl/operation/dictionaryset.cpp
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
    src/networkprotocoldsl/operation/statemachineoperation.cpp
    src/networkprotocoldsl/operation/statemachineoperation.hpp
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
    src/networkprotocoldsl/operation/writeoctetswithescape.cpp
    src/networkprotocoldsl/operation/writeoctetswithescape.hpp
    src/networkprotocoldsl/operation/writestaticoctets.cpp
    src/networkprotocoldsl/operation/writestaticoctets.hpp
    src/networkprotocoldsl/operation/lexicalpadasdict.cpp
    src/networkprotocoldsl/operation/lexicalpadasdict.hpp
    src/networkprotocoldsl/operation/transitionlookahead.cpp
    src/networkprotocoldsl/operation/transitionlookahead.hpp
    src/networkprotocoldsl/parser/grammar/identifier.hpp
    src/networkprotocoldsl/parser/grammar/identifier.cpp
    src/networkprotocoldsl/parser/grammar/literals.hpp
    src/networkprotocoldsl/parser/grammar/literals.cpp
    src/networkprotocoldsl/parser/grammar/message.hpp
    src/networkprotocoldsl/parser/grammar/message.cpp
    src/networkprotocoldsl/parser/grammar/messagedata.hpp
    src/networkprotocoldsl/parser/grammar/messagedata.cpp
    src/networkprotocoldsl/parser/grammar/messagesequence.hpp
    src/networkprotocoldsl/parser/grammar/messagesequence.cpp
    src/networkprotocoldsl/parser/grammar/tokenparts.hpp
    src/networkprotocoldsl/parser/grammar/tokenparts.cpp
    src/networkprotocoldsl/parser/grammar/traits.hpp
    src/networkprotocoldsl/parser/grammar/traits.cpp
    src/networkprotocoldsl/parser/grammar/typeparameter.hpp
    src/networkprotocoldsl/parser/grammar/typeparameter.cpp
    src/networkprotocoldsl/parser/parse.hpp
    src/networkprotocoldsl/parser/parse.cpp
    src/networkprotocoldsl/parser/support/recursiveparser.cpp
    src/networkprotocoldsl/parser/support/recursiveparser.hpp
    src/networkprotocoldsl/parser/tree/booleanliteral.cpp
    src/networkprotocoldsl/parser/tree/booleanliteral.hpp
    src/networkprotocoldsl/parser/tree/identifierreference.cpp
    src/networkprotocoldsl/parser/tree/identifierreference.hpp
    src/networkprotocoldsl/parser/tree/integerliteral.cpp
    src/networkprotocoldsl/parser/tree/integerliteral.hpp
    src/networkprotocoldsl/parser/tree/message.cpp
    src/networkprotocoldsl/parser/tree/messagedata.cpp
    src/networkprotocoldsl/parser/tree/messagedata.hpp
    src/networkprotocoldsl/parser/tree/messageforloop.cpp
    src/networkprotocoldsl/parser/tree/messageforloop.hpp
    src/networkprotocoldsl/parser/tree/message.hpp
    src/networkprotocoldsl/parser/tree/messagepart.cpp
    src/networkprotocoldsl/parser/tree/messagepart.hpp
    src/networkprotocoldsl/parser/tree/messagesequence.cpp
    src/networkprotocoldsl/parser/tree/messagesequence.hpp
    src/networkprotocoldsl/parser/tree/protocoldescription.cpp
    src/networkprotocoldsl/parser/tree/protocoldescription.hpp
    src/networkprotocoldsl/parser/tree/stringliteral.cpp
    src/networkprotocoldsl/parser/tree/stringliteral.hpp
    src/networkprotocoldsl/parser/tree/terminator.cpp
    src/networkprotocoldsl/parser/tree/terminator.hpp
    src/networkprotocoldsl/parser/tree/tokenpart.cpp
    src/networkprotocoldsl/parser/tree/tokenpart.hpp
    src/networkprotocoldsl/parser/tree/tokensequence.cpp
    src/networkprotocoldsl/parser/tree/tokensequence.hpp
    src/networkprotocoldsl/parser/tree/type.cpp
    src/networkprotocoldsl/parser/tree/type.hpp
    src/networkprotocoldsl/parser/tree/typeparametermap.cpp
    src/networkprotocoldsl/parser/tree/typeparametermap.hpp
    src/networkprotocoldsl/parser/tree/typeparametervalue.cpp
    src/networkprotocoldsl/parser/tree/typeparametervalue.hpp
    src/networkprotocoldsl/sema/analyze.cpp
    src/networkprotocoldsl/sema/analyze.hpp
    src/networkprotocoldsl/sema/partstoreadactions.cpp
    src/networkprotocoldsl/sema/partstoreadactions.hpp
    src/networkprotocoldsl/sema/partstowriteactions.cpp
    src/networkprotocoldsl/sema/partstowriteactions.hpp
    src/networkprotocoldsl/sema/support.cpp
    src/networkprotocoldsl/sema/support.hpp
    src/networkprotocoldsl/sema/ast/action.cpp
    src/networkprotocoldsl/sema/ast/action.hpp
    src/networkprotocoldsl/sema/ast/action/loop.cpp
    src/networkprotocoldsl/sema/ast/action/loop.hpp
    src/networkprotocoldsl/sema/ast/action/read.cpp
    src/networkprotocoldsl/sema/ast/action/read.hpp
    src/networkprotocoldsl/sema/ast/action/write.cpp
    src/networkprotocoldsl/sema/ast/action/write.hpp
    src/networkprotocoldsl/sema/ast/agent.cpp
    src/networkprotocoldsl/sema/ast/agent.hpp
    src/networkprotocoldsl/sema/ast/protocol.hpp
    src/networkprotocoldsl/sema/ast/state.cpp
    src/networkprotocoldsl/sema/ast/state.hpp
    src/networkprotocoldsl/sema/ast/transition.cpp
    src/networkprotocoldsl/sema/ast/transition.hpp
    src/networkprotocoldsl/codegen/cppgenerator.cpp
    src/networkprotocoldsl/codegen/cppgenerator.hpp
    src/networkprotocoldsl/codegen/generate_data_types.cpp
    src/networkprotocoldsl/codegen/generate_data_types.hpp
    src/networkprotocoldsl/codegen/generate_parser.cpp
    src/networkprotocoldsl/codegen/generate_parser.hpp
    src/networkprotocoldsl/codegen/generate_runner.cpp
    src/networkprotocoldsl/codegen/generate_runner.hpp
    src/networkprotocoldsl/codegen/generate_serializer.cpp
    src/networkprotocoldsl/codegen/generate_serializer.hpp
    src/networkprotocoldsl/codegen/generate_state_machine.cpp
    src/networkprotocoldsl/codegen/generate_state_machine.hpp
    src/networkprotocoldsl/codegen/generate_states.cpp
    src/networkprotocoldsl/codegen/generate_states.hpp
    src/networkprotocoldsl/codegen/outputcontext.cpp
    src/networkprotocoldsl/codegen/outputcontext.hpp
    src/networkprotocoldsl/codegen/protocolinfo.cpp
    src/networkprotocoldsl/codegen/protocolinfo.hpp
    src/networkprotocoldsl/codegen/typemapping.cpp
    src/networkprotocoldsl/codegen/typemapping.hpp
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
    src/networkprotocoldsl/print_optreenode.cpp
    src/networkprotocoldsl/print_optreenode.hpp
    src/networkprotocoldsl/value.cpp
    src/networkprotocoldsl/value.hpp
    src/networkprotocoldsl/generate.cpp
    src/networkprotocoldsl/generate.hpp
)

target_link_libraries( networkprotocoldsl PUBLIC lexertl )

target_include_directories(
    networkprotocoldsl
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

# Create a separate target for the libuv adapter so users are not forced to use it.
add_library( networkprotocoldsl_uv
    src/networkprotocoldsl_uv/libuvserverrunner.cpp
    src/networkprotocoldsl_uv/libuvserverrunner.hpp
    src/networkprotocoldsl_uv/libuvserverwrapper.cpp   # new file added
    src/networkprotocoldsl_uv/libuvserverwrapper.hpp    # new file added
    src/networkprotocoldsl_uv/libuvclientrunner.cpp
    src/networkprotocoldsl_uv/libuvclientrunner.hpp
    src/networkprotocoldsl_uv/libuvclientwrapper.cpp    # new file added
    src/networkprotocoldsl_uv/libuvclientwrapper.hpp     # new file added
    src/networkprotocoldsl_uv/asyncworkqueue.cpp
    src/networkprotocoldsl_uv/asyncworkqueue.hpp
    src/networkprotocoldsl_uv/generatedserverwrapper.cpp  # wrapper for generated code
    src/networkprotocoldsl_uv/generatedserverwrapper.hpp
)
target_include_directories( networkprotocoldsl_uv PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${LIBUV_INCLUDE_DIR}
)
target_link_libraries( networkprotocoldsl_uv PUBLIC networkprotocoldsl ${LIBUV_LIBRARIES} )

# Protocol generator CLI tool
find_package(CLI11 REQUIRED)
add_executable(protocol_generator
    src/networkprotocoldsl/codegen/protocol_generator.cpp
)
target_link_libraries(protocol_generator PRIVATE networkprotocoldsl CLI11::CLI11)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
