# Building Programming Language Interpreters

**A comprehensive guide to building domain-specific language interpreters and code generators**

This repository contains a Domain-Specific Language (DSL) for network protocol definitions, featuring both an interpreter and a code generator that produces efficient C++ implementations.

## Overview

The project implements **Network Protocol DSL** (`networkprotocoldsl`) — a language for declaratively describing network protocols like SMTP, HTTP, and others. The implementation showcases:

- **Lexer** using lexertl17 for tokenization
- **Recursive descent parser** with template metaprogramming
- **Semantic analyzer** transforming parse trees to Abstract Syntax Trees
- **Interpreter** with continuation-based execution model
- **Code generator** producing efficient C++ code
- **Sans-IO architecture** separating I/O from protocol logic
- **libuv integration** for async I/O

## Architecture

### High-Level Components

```
┌─────────────────────────────────────────────────────────────────┐
│                    Network Protocol DSL                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────────┐ │
│  │  Lexer   │ → │  Parser  │ → │   Sema   │ → │  Interpreter │ │
│  │(lexertl) │   │(Recursive│   │ Analysis │   │  (OpTree)    │ │
│  │          │   │ Descent) │   │          │   │              │ │
│  └──────────┘   └──────────┘   └──────────┘   └──────────────┘ │
│                                      │                          │
│                                      ↓                          │
│                              ┌──────────────┐                   │
│                              │Code Generator│                   │
│                              │   (C++)      │                   │
│                              └──────────────┘                   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Directory Structure

```
src/
├── networkprotocoldsl/           # Core DSL library
│   ├── lexer/                    # Tokenization (lexertl-based)
│   ├── parser/                   # Recursive descent parser
│   │   ├── grammar/              # Grammar rules
│   │   ├── tree/                 # Parse tree node types
│   │   └── support/              # Parser utilities
│   ├── sema/                     # Semantic analysis
│   │   └── ast/                  # Abstract Syntax Tree types
│   ├── codegen/                  # C++ code generation
│   ├── operation/                # Interpreter operations
│   └── support/                  # Utilities (queues, signals)
│
└── networkprotocoldsl_uv/        # libuv async I/O integration
    ├── libuvserverrunner.*       # Server socket handling
    ├── libuvclientrunner.*       # Client socket handling
    └── asyncworkqueue.*          # Async work scheduling

examples/
├── smtpserver/                   # SMTP server (interpreted)
└── smtpserver_generated/         # SMTP server (generated code)

tests/                            # Comprehensive test suite
```

## The DSL Language

The Network Protocol DSL describes protocols as state machines with typed messages. Here's an example for HTTP:

```
message "HTTP Request" { 
    when: Open; 
    then: AwaitResponse; 
    agent: Client; 
    data: { 
        verb: str<encoding=Ascii7Bit, sizing=Dynamic, max_length=10>; 
        request_target: str<encoding=Ascii7Bit, sizing=Dynamic, max_length=32768>; 
        major_version: int<encoding=AsciiInt, unsigned=True, bits=8>; 
        minor_version: int<encoding=AsciiInt, unsigned=True, bits=8>; 
        headers: array<
            element_type=tuple<
                key=str<encoding=Ascii7Bit, sizing=Dynamic, max_length=32768>, 
                value=str<encoding=Ascii7Bit, sizing=Dynamic, max_length=32768>
            >, 
            sizing=Dynamic, 
            max_length=100
        >; 
    } 
    parts { 
        tokens { verb } 
        terminator { " " } 
        tokens { request_target } 
        terminator { " " } 
        tokens { "HTTP/" major_version "." minor_version } 
        terminator { "\r\n" } 
        for header in headers { 
            tokens { header.key } 
            terminator { ": " } 
            tokens<terminator="\r\n", escape=replace<"\n", "\r\n ">> { header.value } 
        } 
        terminator { "\r\n" } 
    } 
}
```

### Language Features

- **State machine definitions**: `when:` / `then:` specify state transitions
- **Agent specification**: `Client` or `Server` role
- **Typed data fields**: `int`, `str`, `array`, `tuple` with parameters
- **Wire format specification**: `tokens`, `terminator`, `for` loops
- **Escape sequences**: `escape=replace<"\n", "\r\n ">` for encoding transformations

## Core Components Explained

### 1. Lexer (`src/networkprotocoldsl/lexer/`)

Uses **lexertl17** to tokenize DSL source code into:
- **Keywords**: `for`, `in`, `message`, `parts`, `terminator`, `tokens`
- **Literals**: `Integer`, `String`, `Boolean`
- **Punctuation**: `<`, `>`, `,`, `{`, `}`, `=`, `:`, `;`, `.`
- **Identifiers**: `[a-zA-Z][a-zA-Z0-9_]*`

### 2. Parser (`src/networkprotocoldsl/parser/`)

Implements a **recursive descent parser** with C++ template metaprogramming:

```cpp
// Example: Parsing types like str<encoding=Ascii7Bit, sizing=Dynamic>
auto parse_result = networkprotocoldsl::parser::parse(tokens);
```

The parser produces a `ProtocolDescription` containing `Message` objects with:
- Message name and state transitions
- Typed data fields (`MessageData`)
- Wire format parts (`MessageSequence`)

### 3. Semantic Analyzer (`src/networkprotocoldsl/sema/`)

Transforms the parse tree into an Abstract Syntax Tree suitable for interpretation and code generation:

```cpp
struct Protocol {
    std::shared_ptr<const Agent> client;  // Client-side state machine
    std::shared_ptr<const Agent> server;  // Server-side state machine
};

struct Agent {
    std::unordered_map<std::string, std::shared_ptr<const State>> states;
};

struct State {
    std::unordered_map<std::string, std::pair<Transition, std::string>> transitions;
};
```

### 4. Interpreter (`src/networkprotocoldsl/`)

The interpreter uses an **Operation Tree (OpTree)** model with:

- **Value types**: `bool`, `int32_t`, `Callable`, `DynamicList`, `Octets`, `Dictionary`
- **Operations**: Add, Subtract, If, FunctionCall, ReadStaticOctets, WriteOctets, etc.
- **Continuation-based execution**: Supports blocking on I/O and callbacks

```cpp
// Creating an interpreted program
auto program = InterpretedProgram::generate_server(source_file);
Interpreter interpreter = program.get_instance();

// Stepping through execution
ContinuationState state = interpreter.step();
if (state == ContinuationState::Blocked) {
    // Handle I/O or callback
}
```

### 5. Sans-IO Architecture

The interpreter separates I/O handling from protocol logic:

1. **IO Thread** - Handles raw byte I/O (sockets)
2. **Protocol Parser Thread** - Steps through interpreter
3. **Callback Handler Thread** - Executes business logic callbacks

```cpp
// Blocking reasons for sans-IO
enum class ReasonForBlockedOperation {
    WaitingForCallback,      // Need callback result
    WaitingCallbackData,     // Waiting for callback data
    WaitingForRead,          // Need more input bytes
    WaitingForWrite,         // Output buffer ready
    WaitingForCallableInvocation,
    WaitingForCallableResult,
};
```

### 6. Code Generator (`src/networkprotocoldsl/codegen/`)

Generates efficient C++ code from the protocol AST:

```bash
# Usage
protocol_generator input.networkprotocoldsl -n myapp::smtp -o generated/
```

Generated files:
- `protocol.hpp` - Main include header
- `data_types.hpp` - Data structs for each message
- `states.hpp` - State enum and transition types
- `parser.hpp` - Sans-IO protocol parser
- `serializer.hpp` - Sans-IO protocol serializer
- `state_machine.hpp` - State machine coordinator

## Building

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+)
- CMake 3.25+
- libuv
- CLI11 (for protocol_generator tool)

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running Tests

```bash
cd build
ctest -j8
```

## Usage Examples

### Interpreted Mode

```cpp
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>

// Load protocol from DSL file
auto program = InterpretedProgram::generate_server("smtp.networkprotocoldsl");

// Set up callback handlers
InterpreterRunner runner{
    .callbacks = {
        {"Open", [](const std::vector<Value>& args) -> Value {
            // Handle new connection, return greeting
            return value::DynamicList{{_o("SMTP Server Greeting"), 
                                       value::Dictionary{...}}};
        }},
        // ... more state handlers
    },
    .exit_when_done = true
};

// Create interpreter collection for managing connections
InterpreterCollectionManager mgr;
mgr.insert_interpreter(client_fd, program);
```

### Generated Code Mode

```cpp
#include "smtp/protocol.hpp"

// Create handler implementing on_<State>() methods
class MyHandler {
public:
    smtp::generated::OpenOutput on_Open() const {
        // Return greeting message
    }
    
    smtp::generated::AwaitServerEHLOResponseOutput 
    on_AwaitServerEHLOResponse(const smtp::generated::SMTPEHLOCommandData& msg) const {
        // Handle EHLO command
    }
    // ... more handlers
};

// Use with generated state machine
MyHandler handler(config);
smtp::generated::ServerStateMachine<MyHandler> machine(handler);
```

### libuv Integration

```cpp
#include <networkprotocoldsl_uv/libuvserverrunner.hpp>

uv_loop_t* loop = uv_default_loop();
AsyncWorkQueue async_queue(loop);

LibuvServerRunner server(
    mgr,           // InterpreterCollectionManager
    loop,          // libuv event loop
    "0.0.0.0",     // bind address
    2525,          // port
    program,       // InterpretedProgram
    async_queue    // async work queue
);

// Server automatically accepts connections and runs interpreters
```

## Test Suite

The test suite (`tests/`) demonstrates incremental development:

| Test | Description |
|------|-------------|
| 001-empty | Basic test infrastructure |
| 002-interpreter-state | Interpreter state management |
| 003-literal-and-add-operations | Basic operations |
| 009-recursive-factorial | Recursive function calls |
| 014-using-with-libuv | libuv I/O integration |
| 015-test-tokenizer | Lexer testing |
| 016-023 | Parser grammar tests |
| 024-sema-analyze | Semantic analysis |
| 027-translate-ast-to-optree | AST to OpTree translation |
| 028-libuv-io-runner | Full I/O runner tests |
| 029-039 | Code generation tests |

## Key Design Decisions

### 1. Immutable Data Structures

Values use `std::shared_ptr<const T>` for safe sharing:

```cpp
struct Octets {
    std::shared_ptr<const std::string> data;
};

struct Dictionary {
    std::shared_ptr<const std::unordered_map<std::string, Value>> members;
};
```

### 2. Type-Safe Variant Operations

Operations use C++ concepts for compile-time safety:

```cpp
template <typename OT>
concept InputOutputOperationConcept = requires(OT op, ...) {
    { op(ctx, args) } -> std::convertible_to<OperationResult>;
    { op.handle_read(ctx, sv) } -> std::convertible_to<std::size_t>;
    { op.get_write_buffer(ctx) } -> std::convertible_to<std::string_view>;
    // ...
};
```

### 3. Continuation-Based Execution

The interpreter can pause and resume execution:

```cpp
class Continuation {
    std::stack<ExecutionStackFrame> stack;
    ContinuationState state;
    
    ContinuationState step();  // Execute one operation
    void handle_read(std::string_view in);
    std::string_view get_write_buffer();
};
```

### 4. Thread Separation

Clean interfaces between threads via notification signals:

```cpp
struct InterpreterCollectionSignals {
    NotificationSignal wake_up_for_input;
    NotificationSignal wake_up_for_output;
    NotificationSignal wake_up_for_callback;
    NotificationSignal wake_up_interpreter;
};
```

## License

See [LICENSE](LICENSE) file for details.


