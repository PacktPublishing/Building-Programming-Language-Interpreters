# Code Generation Plan: Network Protocol DSL to C++

## Executive Summary

This document outlines the plan to translate the Network Protocol DSL into compile-time C++ code while preserving the sans-IO architecture. The generated code will maintain the separation between:

1. **IO Thread** - Handles raw byte I/O operations (reading/writing from sockets)
2. **Protocol Parser Thread** - Handles parsing of the protocol according to the DSL definition
3. **Callback Handler Thread** - Handles state transition callbacks with business logic

## Current Architecture Analysis

### Lexer (src/networkprotocoldsl/lexer/)

The lexer uses `lexertl` to tokenize the DSL source code into the following token types:

**Keywords:**
- `for`, `in`, `message`, `parts`, `terminator`, `tokens`

**Literals:**
- `Integer` (e.g., `123`)
- `String` (e.g., `"hello\n"`)
- `Boolean` (`True`, `False`)

**Punctuation:**
- `<`, `>`, `,`, `{`, `}`, `=`, `:`, `;`, `.`

**Identifiers:**
- Names matching `[a-zA-Z][a-zA-Z0-9_]*`

### Parser (src/networkprotocoldsl/parser/)

The parser uses a recursive descent approach with template metaprogramming (`RecursiveParser`). Key parse tree structures:

**ProtocolDescription** - A map of message names to Message objects

**Message** - Contains:
- `name` - StringLiteral
- `when` - IdentifierReference (source state)
- `then` - IdentifierReference (target state)
- `agent` - IdentifierReference ("Client" or "Server")
- `data` - MessageData (typed fields)
- `parts` - MessageSequence (wire format specification)

**MessageData** - Map of field names to Type definitions

**Type** - Contains:
- `name` - IdentifierReference (e.g., `int`, `str`, `array`, `tuple`)
- `parameters` - TypeParameterMap (e.g., `encoding=Ascii7Bit`, `max_length=256`)

**MessageSequence** - Vector of MessagePart variants:
- `TokenSequence` - Sequence of string literals and identifier references
- `Terminator` - End-of-message marker (e.g., `"\r\n"`)
- `MessageForLoop` - Iteration over array fields

### Semantic Analyzer (src/networkprotocoldsl/sema/)

The semantic analyzer transforms the parse tree into an Abstract Syntax Tree (AST) suitable for code generation:

**Protocol** - Contains client and server Agent objects

**Agent** - Map of state names to State objects

**State** - Contains:
- `transitions` - Map of message names to (Transition, target_state) pairs

**Transition** - Variant of:
- `ReadTransition` - For messages received by this agent
- `WriteTransition` - For messages sent by this agent

**Actions** - Sequence of operations:
- `ReadStaticOctets` - Read fixed bytes
- `ReadOctetsUntilTerminator` - Read until delimiter
- `WriteStaticOctets` - Write fixed bytes
- `WriteFromIdentifier` - Write value from variable
- `Loop` - Iterate over array field

### Current Interpreter Architecture

The interpreter implements sans-IO through several key components:

1. **InterpretedProgram** - Holds the compiled OpTree
2. **Interpreter** - Manages continuation stack for execution
3. **InterpreterRunner** - Coordinates three loops:
   - `interpreter_loop` - Steps interpreter, handles blocked states
   - `callback_loop` - Dispatches callbacks to user code
   - (external) IO loop - Handles actual byte I/O

**Blocking Reasons:**
- `WaitingForRead` - Need more input bytes
- `WaitingForWrite` - Output buffer ready to send
- `WaitingForCallback` - Need callback result from user
- `WaitingCallbackData` - Waiting for callback response

## Generated Code Architecture

### Overview

The code generator takes the following inputs:
- **Protocol DSL file** - The `.networkprotocoldsl` source file
- **Target namespace** - C++ namespace for generated code (e.g., `myapp::smtp`)
- **Target directory** - Output directory for generated files

The generated C++ code will produce a header-only library with **fixed, protocol-independent entry point names**:

```
<target_directory>/
├── protocol.hpp           # Main include, all types and entry points
├── states.hpp             # State enum and transition types
├── data_types.hpp         # Data structs for each message
├── parser.hpp             # Sans-IO protocol parser
├── serializer.hpp         # Sans-IO protocol serializer
└── state_machine.hpp      # State machine coordinator
```

### Fixed Entry Point Names

Regardless of the protocol being generated, the following names are always used within the target namespace:

| Entry Point | Description |
|-------------|-------------|
| `<namespace>::State` | Enum of all protocol states |
| `<namespace>::ClientStateMachine` | State machine for client role |
| `<namespace>::ServerStateMachine` | State machine for server role |
| `<namespace>::Parser` | Sans-IO parser |
| `<namespace>::Serializer` | Sans-IO serializer |

This allows integration code to be written generically:

```cpp
// User code knows the convention, not the protocol specifics
namespace proto = myapp::smtp;  // or myapp::http, myapp::ftp, etc.

proto::ServerStateMachine<MyHandler> machine(handler);
proto::State current = machine.current_state();
```

### Design Principles

1. **Zero Allocation Fast Path** - Pre-allocate buffers, avoid dynamic allocation during parsing
2. **Sans-IO** - All I/O operations return status codes indicating more data needed or output ready
3. **Thread Separation** - Clean interfaces between IO, parsing, and callback threads
4. **Type Safety** - Use C++ type system to ensure correct state transitions

---

## Phase 1: Data Type Generation

### 1.1 Type Mapping

Map DSL types to C++ types:

| DSL Type | C++ Type |
|----------|----------|
| `int<encoding=AsciiInt, unsigned=True, bits=8>` | `uint8_t` |
| `int<encoding=AsciiInt, unsigned=True, bits=16>` | `uint16_t` |
| `int<encoding=AsciiInt, unsigned=True, bits=32>` | `uint32_t` |
| `int<encoding=AsciiInt, unsigned=False, bits=8>` | `int8_t` |
| `str<encoding=Ascii7Bit, sizing=Dynamic, max_length=N>` | `std::string` |
| `str<encoding=Ascii7Bit, sizing=Fixed, length=N>` | `std::array<char, N>` |
| `array<element_type=T, sizing=Dynamic>` | `std::vector<T>` |
| `tuple<field1=T1, field2=T2, ...>` | `struct { T1 field1; T2 field2; ... }` |

### 1.2 Message Data Structs

For each message, generate a data struct:

```cpp
// For "SMTP EHLO Command"
struct SMTPEHLOCommandData {
    std::string client_domain;
    
    // Move semantics for efficient transfer
    SMTPEHLOCommandData() = default;
    SMTPEHLOCommandData(SMTPEHLOCommandData&&) = default;
    SMTPEHLOCommandData& operator=(SMTPEHLOCommandData&&) = default;
};
```

### 1.3 Transition Data Types

Generate variant types for transitions from each state:

```cpp
// Transitions from ClientSendEHLO state
using ClientSendEHLOTransition = std::variant<
    SMTPEHLOCommandData,        // -> AwaitServerEHLOResponse
    SMTPQUITCommandFromEHLOData // -> AwaitServerQUITResponse
>;
```

### Tasks

- [ ] **1.1.1** Create type mapping utility functions
- [ ] **1.1.2** Handle nested types (tuple, array of tuples)
- [ ] **1.1.3** Generate type parameter extraction for encoding info
- [ ] **1.2.1** Generate message data struct for each message
- [ ] **1.2.2** Add constructors and move semantics
- [ ] **1.2.3** Add serialization helpers
- [ ] **1.3.1** Analyze state machine to find all transitions from each state
- [ ] **1.3.2** Generate variant types for transition data

---

## Phase 2: State Machine Generation

### 2.1 State Enumeration

```cpp
enum class ProtocolState {
    Open,
    ClientSendEHLO,
    AwaitServerEHLOResponse,
    ClientSendCommand,
    // ... all states
    Closed
};
```

### 2.2 State Traits

Generate compile-time traits for each state:

```cpp
template<ProtocolState S>
struct StateTraits;

template<>
struct StateTraits<ProtocolState::ClientSendEHLO> {
    using Agent = Client;
    using IncomingDataType = void; // No incoming data expected
    using OutgoingTransitionType = ClientSendEHLOTransition;
    static constexpr bool is_terminal = false;
};

template<>
struct StateTraits<ProtocolState::AwaitServerEHLOResponse> {
    using Agent = Server;
    using IncomingDataType = AwaitServerEHLOResponseIncoming;
    using OutgoingTransitionType = void; // Callback provides transition
    static constexpr bool is_terminal = false;
};
```

### Tasks

- [ ] **2.1.1** Extract all unique state names from protocol
- [ ] **2.1.2** Generate state enum
- [ ] **2.2.1** Determine agent (Client/Server) for each state based on message definitions
- [ ] **2.2.2** Generate StateTraits for each state
- [ ] **2.2.3** Link states to their possible transitions

---

## Phase 3: Parser Generation

### 3.1 Sans-IO Parser Interface

```cpp
class ProtocolParser {
public:
    enum class ParseResult {
        NeedMoreData,      // Not enough bytes to parse
        Complete,          // Message parsed successfully
        Error              // Protocol error
    };
    
    // Feed bytes to the parser
    // Returns number of bytes consumed
    size_t feed(std::string_view input);
    
    // Check if a complete message is available
    ParseResult status() const;
    
    // Get the parsed message (only valid when status() == Complete)
    // Returns the message name and data
    std::pair<std::string, MessageDataVariant> take_message();
    
    // Current state
    ProtocolState current_state() const;
};
```

### 3.2 Read Action Generation

For each read transition, generate parsing code:

```cpp
// Generated for "SMTP EHLO Command"
class SMTPEHLOCommandParser {
private:
    enum class Stage { ReadEHLO, ReadSpace, ReadClientDomain, ReadTerminator, Done };
    Stage stage_ = Stage::ReadEHLO;
    std::string client_domain_;
    
public:
    struct ParseResult {
        size_t consumed;
        bool complete;
        bool error;
    };
    
    ParseResult parse(std::string_view input) {
        size_t total_consumed = 0;
        
        switch (stage_) {
        case Stage::ReadEHLO:
            // Match static octets "EHLO "
            if (input.size() < 5) return {0, false, false};
            if (input.substr(0, 5) != "EHLO ") return {0, false, true};
            total_consumed += 5;
            input.remove_prefix(5);
            stage_ = Stage::ReadClientDomain;
            [[fallthrough]];
            
        case Stage::ReadClientDomain:
            // Read until terminator "\r\n"
            {
                auto pos = input.find("\r\n");
                if (pos == std::string_view::npos) {
                    // Buffer partial data
                    client_domain_.append(input);
                    return {total_consumed + input.size(), false, false};
                }
                client_domain_.append(input.substr(0, pos));
                total_consumed += pos + 2;
                stage_ = Stage::Done;
            }
            break;
            
        case Stage::Done:
            break;
        }
        
        return {total_consumed, true, false};
    }
    
    SMTPEHLOCommandData take_data() {
        SMTPEHLOCommandData data;
        data.client_domain = std::move(client_domain_);
        return data;
    }
};
```

### 3.3 Transition Lookahead Generation

Generate lookahead logic for states with multiple possible incoming messages:

```cpp
class AwaitServerEHLOResponseLookahead {
public:
    enum class Match {
        Unknown,           // Need more data
        SuccessResponse,   // Matches "2XX ..."
        FailureResponse    // Matches "5XX ..."
    };
    
    Match check(std::string_view input) const {
        if (input.size() < 1) return Match::Unknown;
        
        if (input[0] == '2') return Match::SuccessResponse;
        if (input[0] == '5') return Match::FailureResponse;
        
        return Match::Unknown; // Could be an error or need more data
    }
};
```

### Tasks

- [ ] **3.1.1** Define ParseResult enum and parser interface
- [ ] **3.1.2** Implement base parser class with state management
- [ ] **3.2.1** Generate static octet matching code
- [ ] **3.2.2** Generate read-until-terminator code with buffering
- [ ] **3.2.3** Generate integer parsing from ASCII
- [ ] **3.2.4** Generate loop parsing for arrays
- [ ] **3.2.5** Handle nested tuple parsing in loops
- [ ] **3.3.1** Analyze first bytes of each possible incoming message
- [ ] **3.3.2** Generate lookahead matchers for ambiguous states
- [ ] **3.3.3** Handle EOF as a transition condition

---

## Phase 4: Serializer Generation

### 4.1 Sans-IO Serializer Interface

```cpp
class ProtocolSerializer {
public:
    // Serialize a transition
    // Returns bytes to write
    std::string serialize(const MessageDataVariant& data);
    
    // Check how many bytes have been written
    size_t bytes_pending() const;
    
    // Mark bytes as written
    void consume(size_t bytes);
    
    // Get the next chunk to write
    std::string_view get_write_buffer() const;
};
```

### 4.2 Write Action Generation

For each write transition, generate serialization code:

```cpp
// Generated for "SMTP Server Greeting"
std::string serialize_SMTPServerGreeting(const SMTPServerGreetingData& data) {
    std::string result;
    result.reserve(128); // Pre-allocate reasonable size
    
    // Static octets
    result += "2";
    
    // Integer to ASCII
    result += std::to_string(data.code_tens);
    
    // Static octets
    result += " ";
    
    // String field
    result += data.msg;
    
    // Terminator
    result += "\r\n";
    
    return result;
}
```

### 4.3 Loop Serialization

For messages with loops (e.g., array of recipients):

```cpp
std::string serialize_MessageWithLoop(const MessageWithLoopData& data) {
    std::string result;
    
    for (const auto& item : data.items) {
        result += item.field1;
        result += " ";
    }
    result += "\r\n"; // Loop terminator
    
    return result;
}
```

### Tasks

- [ ] **4.1.1** Define serializer interface
- [ ] **4.2.1** Generate static octet writing code
- [ ] **4.2.2** Generate integer-to-ASCII conversion
- [ ] **4.2.3** Generate string field writing
- [ ] **4.3.1** Generate loop serialization
- [ ] **4.3.2** Handle nested tuples in loops
- [ ] **4.3.3** Add terminator after loops

---

## Phase 5: State Machine Coordinator Generation

### 5.1 Coordinator Interface

```cpp
template<typename CallbackHandler>
class ProtocolStateMachine {
public:
    ProtocolStateMachine(CallbackHandler& handler);
    
    // Sans-IO interface for the IO thread
    struct IOStatus {
        bool need_read;
        bool has_write;
        bool is_closed;
    };
    
    IOStatus get_io_status() const;
    
    // Feed input bytes (called from IO thread)
    size_t on_input(std::string_view data);
    
    // Get output to write (called from IO thread)
    std::string_view get_output();
    void output_consumed(size_t bytes);
    
    // Handle EOF
    void on_eof();
    
    // Interface for callback thread
    bool has_pending_callback() const;
    void process_callbacks();
};
```

### 5.2 Callback Handler Interface

```cpp
// User must implement this interface
class CallbackHandler {
public:
    // Called when entering a new state that requires a callback
    // Returns the transition data to be sent
    template<ProtocolState S>
    typename StateTraits<S>::OutgoingTransitionType 
    on_state_entered(std::unique_ptr<typename StateTraits<S>::IncomingDataType> data);
    
    // Alternatively, use a variant-based interface:
    TransitionVariant on_state_entered(ProtocolState state, 
                                       std::unique_ptr<IncomingDataVariant> data);
};
```

### 5.3 Thread Safety

```cpp
class ProtocolStateMachine {
private:
    // IO thread accesses these
    std::atomic<IOFlags> io_flags_;
    RingBuffer<char> input_buffer_;
    RingBuffer<char> output_buffer_;
    
    // Parser thread accesses these
    ProtocolState current_state_;
    std::variant<...parsers...> active_parser_;
    
    // Synchronization
    std::mutex callback_mutex_;
    std::condition_variable callback_cv_;
    std::queue<std::pair<ProtocolState, IncomingDataVariant>> pending_callbacks_;
    std::queue<TransitionVariant> callback_responses_;
    
public:
    // Called from IO thread
    size_t on_input(std::string_view data);
    
    // Called from parser thread
    void parser_loop();
    
    // Called from callback thread
    void callback_loop(CallbackHandler& handler);
};
```

### Tasks

- [ ] **5.1.1** Define IOStatus struct
- [ ] **5.1.2** Implement input buffer management
- [ ] **5.1.3** Implement output buffer management
- [ ] **5.2.1** Define callback handler concept/interface
- [ ] **5.2.2** Generate state-specific callback signatures
- [ ] **5.2.3** Generate transition routing based on callback return
- [ ] **5.3.1** Add synchronization primitives
- [ ] **5.3.2** Implement callback queue
- [ ] **5.3.3** Implement response queue

---

## Phase 6: Code Generator Implementation

### 6.1 Generator Architecture

```cpp
namespace codegen {

class CppGenerator {
public:
    /**
     * @param protocol The analyzed protocol AST
     * @param target_namespace The C++ namespace for generated code (e.g., "myapp::smtp")
     * @param target_directory The output directory for generated files
     */
    CppGenerator(const sema::ast::Protocol& protocol, 
                 const std::string& target_namespace,
                 const std::filesystem::path& target_directory);
    
    // Generate all files
    void generate();
    
private:
    std::string target_namespace_;
    std::filesystem::path target_directory_;
    
    // Individual file generators
    std::string generate_data_types();
    std::string generate_states();
    std::string generate_parsers();
    std::string generate_serializers();
    std::string generate_state_machine();
    std::string generate_main_header();
    
    // Helper methods
    std::string type_to_cpp(const parser::tree::Type& type);
    std::string generate_parse_action(const sema::ast::Action& action);
    std::string generate_write_action(const sema::ast::Action& action);
    
    // Namespace helpers
    std::string open_namespace() const;   // "namespace myapp::smtp {"
    std::string close_namespace() const;  // "} // namespace myapp::smtp"
};

} // namespace codegen
```

### 6.2 Command Line Interface

```
protocol_generator <input.networkprotocoldsl> --namespace <namespace> --output <directory>

Examples:
  protocol_generator smtp.networkprotocoldsl --namespace myapp::smtp --output generated/smtp
  protocol_generator http.networkprotocoldsl --namespace protocols::http --output src/generated/http
```

### 6.3 Integration with Build System

CMake integration:

```cmake
# Custom command to generate protocol code
add_custom_command(
    OUTPUT ${TARGET_DIR}/protocol.hpp
    COMMAND protocol_generator 
            ${PROTOCOL_DSL_FILE} 
            --namespace ${TARGET_NAMESPACE}
            --output ${TARGET_DIR}
    DEPENDS ${PROTOCOL_DSL_FILE} protocol_generator
    COMMENT "Generating C++ code for ${TARGET_NAMESPACE}"
)

# Generated library
add_library(${TARGET_NAME}_generated INTERFACE)
target_include_directories(${TARGET_NAME}_generated 
    INTERFACE ${TARGET_DIR}/..)  # Parent so includes work as <dir/protocol.hpp>
```

Example usage in a project:

```cmake
# Generate SMTP protocol code
set(SMTP_NAMESPACE "myapp::protocols::smtp")
set(SMTP_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated/smtp")

add_custom_command(
    OUTPUT ${SMTP_OUTPUT_DIR}/protocol.hpp
    COMMAND protocol_generator 
            ${CMAKE_SOURCE_DIR}/protocols/smtp.networkprotocoldsl
            --namespace ${SMTP_NAMESPACE}
            --output ${SMTP_OUTPUT_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/protocols/smtp.networkprotocoldsl
)

add_library(smtp_protocol INTERFACE)
target_sources(smtp_protocol INTERFACE ${SMTP_OUTPUT_DIR}/protocol.hpp)
target_include_directories(smtp_protocol INTERFACE ${CMAKE_BINARY_DIR}/generated)

# Usage: #include <smtp/protocol.hpp>
# Access: myapp::protocols::smtp::ServerStateMachine
```

### Tasks

- [ ] **6.1.1** Create CppGenerator class skeleton with namespace/directory parameters
- [ ] **6.1.2** Implement namespace opening/closing helpers
- [ ] **6.1.3** Implement type_to_cpp mapping
- [ ] **6.1.4** Implement data types generation
- [ ] **6.1.5** Implement states generation
- [ ] **6.1.6** Implement parsers generation
- [ ] **6.1.7** Implement serializers generation
- [ ] **6.1.8** Implement state machine generation
- [ ] **6.1.9** Implement main header generation
- [ ] **6.2.1** Create CLI argument parser for generator
- [ ] **6.2.2** Validate namespace format (must be valid C++ nested namespace)
- [ ] **6.2.3** Create output directory if needed
- [ ] **6.3.1** Create CMake module for code generation
- [ ] **6.3.2** Add generator executable target
- [ ] **6.3.3** Test with SMTP example

---

## Phase 7: Testing and Validation

### 7.1 Unit Tests

- [ ] **7.1.1** Test type mapping correctness
- [ ] **7.1.2** Test parser generation for each action type
- [ ] **7.1.3** Test serializer generation for each action type
- [ ] **7.1.4** Test state machine transitions
- [ ] **7.1.5** Test lookahead generation

### 7.2 Integration Tests

- [ ] **7.2.1** Generate code for SMTP protocol
- [ ] **7.2.2** Compare behavior with interpreted version
- [ ] **7.2.3** Test with real SMTP client/server
- [ ] **7.2.4** Verify sans-IO properties (no blocking calls)

### 7.3 Performance Tests

- [ ] **7.3.1** Benchmark parsing speed vs interpreted
- [ ] **7.3.2** Benchmark serialization speed vs interpreted
- [ ] **7.3.3** Memory allocation profiling

---

## Phase 8: Documentation and Examples

### Tasks

- [ ] **8.1.1** Document generated code structure
- [ ] **8.1.2** Document callback handler implementation guide
- [ ] **8.1.3** Document integration with various IO libraries
- [ ] **8.2.1** Create SMTP server example using generated code
- [ ] **8.2.2** Create SMTP client example using generated code
- [ ] **8.2.3** Create example with libuv integration
- [ ] **8.2.4** Create example with asio integration

---

## Appendix A: Generated Code Example (SMTP)

Generated with: `protocol_generator smtp.networkprotocoldsl --namespace smtp::generated --output generated/smtp`

### data_types.hpp

```cpp
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace smtp::generated {

struct SMTPServerGreetingData {
    uint8_t code_tens;
    std::string msg;
};

struct SMTPEHLOCommandData {
    std::string client_domain;
};

struct SMTPEHLOSuccessResponseData {
    std::string client_domain;
    uint8_t code_tens;
    std::string msg;
};

// ... more data types

} // namespace smtp::generated
```

### states.hpp

```cpp
#pragma once
#include <variant>
#include "data_types.hpp"

namespace smtp::generated {

enum class State {
    Open,
    ClientSendEHLO,
    AwaitServerEHLOResponse,
    ClientSendCommand,
    AwaitServerMAILFROMResponse,
    // ...
    Closed
};

// Callback input for AwaitServerEHLOResponse state
using AwaitServerEHLOResponseInput = std::variant<
    SMTPEHLOCommandData
>;

// Callback output for AwaitServerEHLOResponse state
using AwaitServerEHLOResponseOutput = std::variant<
    SMTPEHLOSuccessResponseData,
    SMTPEHLOFailureResponseData
>;

} // namespace smtp::generated
```

### Example Usage

```cpp
#include <smtp/protocol.hpp>  // Generated header
#include <memory>

// The namespace is specified at generation time
namespace proto = smtp::generated;

class SMTPServerHandler {
public:
    // Fixed interface name: on_<StateName>
    // Input: unique_ptr with parsed data
    // Output: variant of possible transitions from this state
    proto::AwaitServerEHLOResponseOutput
    on_AwaitServerEHLOResponse(
        std::unique_ptr<proto::SMTPEHLOCommandData> data) 
    {
        if (is_blocked_domain(data->client_domain)) {
            return proto::SMTPEHLOFailureResponseData{
                .client_domain = std::move(data->client_domain),
                .code_tens = 55,
                .msg = "Bad client, go away!"
            };
        }
        
        return proto::SMTPEHLOSuccessResponseData{
            .client_domain = std::move(data->client_domain),
            .code_tens = 50,
            .msg = "Hello, pleased to meet you"
        };
    }
    
    // ... more state handlers
};

int main() {
    SMTPServerHandler handler;
    
    // Fixed name: ServerStateMachine (or ClientStateMachine)
    proto::ServerStateMachine<SMTPServerHandler> machine(handler);
    
    // Fixed name: State enum
    proto::State current = machine.current_state();
    
    // IO thread pushes data
    machine.on_input("EHLO example.com\r\n");
    
    // Parser thread processes
    machine.parser_step();
    
    // Callback thread handles
    machine.callback_step();
    
    // IO thread gets output
    auto output = machine.get_output();
    // send(socket, output.data(), output.size());
}
```

---

## Appendix B: Fixed Entry Point Convention

All generated protocols follow the same naming convention, allowing generic integration code:

| Generated Name | Type | Description |
|----------------|------|-------------|
| `State` | `enum class` | All protocol states |
| `Parser` | `class` | Sans-IO parser |
| `Serializer` | `class` | Sans-IO serializer |
| `ClientStateMachine<Handler>` | `class template` | Client-side state machine |
| `ServerStateMachine<Handler>` | `class template` | Server-side state machine |
| `<StateName>Input` | `using` (variant) | Possible inputs when entering state |
| `<StateName>Output` | `using` (variant) | Possible outputs (transitions) from state |
| `<MessageName>Data` | `struct` | Data fields for a message |

This allows writing generic protocol wrappers:

```cpp
template<typename Protocol, typename Handler>
class GenericServer {
    typename Protocol::ServerStateMachine<Handler> machine_;
    
public:
    GenericServer(Handler& h) : machine_(h) {}
    
    void on_data(std::string_view data) {
        machine_.on_input(data);
    }
    
    bool is_closed() const {
        return machine_.current_state() == Protocol::State::Closed;
    }
};

// Works with any generated protocol
GenericServer<smtp::generated, SMTPHandler> smtp_server(handler);
GenericServer<http::generated, HTTPHandler> http_server(handler);
```

---

## Appendix C: File List and Dependencies

```
src/
├── networkprotocoldsl/
│   ├── codegen/                    # NEW: Code generation module
│   │   ├── cppgenerator.hpp
│   │   ├── cppgenerator.cpp
│   │   ├── datatypes.hpp           # Data type generation
│   │   ├── datatypes.cpp
│   │   ├── states.hpp              # State enum generation
│   │   ├── states.cpp
│   │   ├── parsers.hpp             # Parser generation
│   │   ├── parsers.cpp
│   │   ├── serializers.hpp         # Serializer generation
│   │   ├── serializers.cpp
│   │   ├── statemachine.hpp        # State machine generation
│   │   └── statemachine.cpp
│   └── ... (existing files)
├── tools/
│   └── protocol_generator/         # NEW: Generator executable
│       ├── main.cpp
│       └── CMakeLists.txt
└── examples/
    └── smtpserver_generated/       # NEW: Generated SMTP example
        ├── CMakeLists.txt
        ├── main.cpp
        └── server_handler.cpp
```

---

## Risk Assessment

| Risk | Mitigation |
|------|------------|
| Complex nested types | Start with simple types, incrementally add complexity |
| Lookahead ambiguity | Analyze protocol for ambiguous states early, fail fast |
| Thread safety bugs | Use well-tested lock-free queues, extensive testing |
| Performance regression | Benchmark early and often, compare with interpreted |
| Edge cases in parsing | Use existing interpreter tests as validation |

---

## Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: Data Types | 1 week | - |
| Phase 2: State Machine | 1 week | Phase 1 |
| Phase 3: Parser | 2 weeks | Phase 1, 2 |
| Phase 4: Serializer | 1 week | Phase 1, 2 |
| Phase 5: Coordinator | 2 weeks | Phase 3, 4 |
| Phase 6: Generator | 2 weeks | Phase 1-5 |
| Phase 7: Testing | 2 weeks | Phase 6 |
| Phase 8: Documentation | 1 week | Phase 7 |

**Total: ~12 weeks**
