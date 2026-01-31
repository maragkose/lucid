# LUCID Compiler - Phase 5: Virtual Machine (Bytecode Execution)

**Status**: ✅ COMPLETE - All Days Implemented!

## Overview
Implement a stack-based virtual machine that executes the bytecode generated in Phase 4. The VM will interpret bytecode instructions, manage the call stack, and execute LUCID programs.

## Context
- **Input**: Bytecode from Phase 4 compiler
- **Output**: Program execution results
- **Architecture**: Stack-based bytecode interpreter
- **Scope**: Pure computation (no IO), core language features + collections
- **Foundation**: Builds on Phase 4's bytecode format and Value system

## Prerequisites (Already Complete from Phase 4)
✅ Bytecode instruction set (34 opcodes)
✅ Value type system (Int, Float, String, Bool, List, Tuple, Function)
✅ Bytecode compiler
✅ Constant pool and function table infrastructure
✅ 207 tests passing for compilation

## File Structure

### New Files to Create
```
include/lucid/backend/
  └── vm.hpp               # Virtual machine interface

src/backend/
  └── vm.cpp               # VM implementation (~800-1200 lines)

tests/
  └── vm_test.cpp          # VM execution tests
```

### Files to Use (Already Exist)
```
include/lucid/backend/
  ├── bytecode.hpp         # Bytecode definitions (from Phase 4)
  ├── value.hpp            # Runtime value representation (from Phase 4)
  └── compiler.hpp         # Bytecode compiler (from Phase 4)

src/backend/
  ├── bytecode.cpp         # Bytecode utilities (from Phase 4)
  ├── value.cpp            # Value operations (from Phase 4)
  └── compiler.cpp         # Compiler implementation (from Phase 4)
```

### Files to Modify
```
CMakeLists.txt            # Add vm_test.cpp (vm.cpp already in build)
```

---

## Design Decisions

### 1. Stack-Based Execution Model

**Stack machine architecture:**
- **Value stack**: Operands and intermediate results
- **Call stack**: Function call frames
- **Instruction pointer (IP)**: Current execution position

**Example execution:**
```python
# LUCID code:
x + y * 2

# Bytecode:
LOAD_LOCAL 0     # x
LOAD_LOCAL 1     # y
CONSTANT 2
MUL
ADD

# Stack evolution:
[]           # Initial
[x]          # After LOAD_LOCAL 0
[x, y]       # After LOAD_LOCAL 1
[x, y, 2]    # After CONSTANT 2
[x, (y*2)]   # After MUL
[(x+y*2)]    # After ADD
```

### 2. Call Frame Structure

Each function call creates a new stack frame:

```cpp
struct CallFrame {
    size_t function_index;      // Which function is executing
    size_t return_address;      // Where to return (IP)
    size_t frame_start;         // Start of locals in value stack
    size_t local_count;         // Number of local variables
};
```

### 3. VM State

```cpp
class VM {
private:
    // Bytecode being executed
    const Bytecode* bytecode_;

    // Value stack (operands, temporaries)
    std::vector<Value> stack_;

    // Call stack (function frames)
    std::vector<CallFrame> frames_;

    // Instruction pointer
    size_t ip_;

    // Current frame
    CallFrame* current_frame_;
};
```

### 4. Instruction Dispatch

Two main strategies:

**A. Switch-based dispatch (simpler, start with this):**
```cpp
auto VM::execute() -> Value {
    while (true) {
        uint8_t opcode = read_byte();

        switch (static_cast<OpCode>(opcode)) {
            case OpCode::CONSTANT: {
                uint16_t index = read_uint16();
                push(bytecode_->constants[index]);
                break;
            }
            case OpCode::ADD: {
                Value b = pop();
                Value a = pop();
                push(a + b);
                break;
            }
            // ... more cases
        }
    }
}
```

**B. Computed goto / jump table (optimization for later):**
- 20-30% faster than switch
- GCC/Clang extension
- Defer until optimization phase

---

## Implementation Plan: 6-Day Breakdown

### Day 1: VM Infrastructure & Basic Execution
**Goal**: Set up VM class and implement simple arithmetic

**Tasks:**
1. Create `include/lucid/backend/vm.hpp`:
   - VM class definition
   - CallFrame structure
   - Execution state management
   - Stack operations (push/pop/peek)

2. Create `src/backend/vm.cpp`:
   - VM constructor/destructor
   - Basic instruction reading (read_byte, read_uint16)
   - Stack manipulation (push, pop, peek, peek_at)
   - Frame management (push_frame, pop_frame)

3. Implement basic opcodes:
   - `CONSTANT` - Push constant from pool
   - `TRUE`, `FALSE` - Push boolean literals
   - `ADD`, `SUB`, `MUL`, `DIV`, `MOD` - Arithmetic
   - `RETURN` - Return from function
   - `HALT` - Stop execution

4. Implement arithmetic operations:
   - Type checking (Int + Int, Float + Float)
   - Type coercion (Int + Float → Float)
   - Error handling for type mismatches

**Files:**
- `include/lucid/backend/vm.hpp` (~150 lines)
- `src/backend/vm.cpp` (start, ~300 lines)
- `tests/vm_test.cpp` (start, ~150 lines)

**Tests:**
- Execute simple constants
- Execute arithmetic expressions
- Type checking and coercion

**Success Criteria:**
```cpp
TEST_CASE("VM: Simple arithmetic") {
    // Compile: 5 + 3 * 2
    // Execute and verify result is 11
}
```

---

### Day 2: Variables & Function Calls
**Goal**: Local variables and function invocation

**Tasks:**
1. Implement local variable opcodes:
   - `LOAD_LOCAL` - Load local variable by index
   - `STORE_LOCAL` - Store to local variable
   - Local variable indexing in frames

2. Implement function call opcodes:
   - `LOAD_GLOBAL` - Load function reference
   - `CALL` - Call function
     - Create new call frame
     - Transfer arguments
     - Set up local variables
     - Jump to function entry point
   - `RETURN` - Return from function
     - Pop call frame
     - Restore previous IP
     - Leave return value on stack

3. Frame management:
   - Maintain frame_start for each call
   - Track local_count for each function
   - Proper stack cleanup on return

**Example execution:**
```python
# LUCID:
function add(x: Int, y: Int) returns Int {
    return x + y
}
add(5, 3)

# Execution:
1. Push arguments: [5, 3]
2. CALL add
   - Create frame: {function_index=0, frame_start=0, local_count=2}
   - Locals: x=5, y=3
3. Execute function body: LOAD_LOCAL 0, LOAD_LOCAL 1, ADD, RETURN
4. Pop frame, result: [8]
```

**Tests:**
- Local variable access
- Simple function calls
- Functions with multiple parameters
- Nested function calls

**Success Criteria:**
```cpp
TEST_CASE("VM: Function calls") {
    // Compile and execute: add(5, 3)
    // Verify result is 8
}
```

---

### Day 3: Comparison & Control Flow
**Goal**: Implement if expressions and comparison operators

**Tasks:**
1. Implement comparison opcodes:
   - `EQ`, `NE` - Equality
   - `LT`, `GT`, `LE`, `GE` - Ordering
   - Type checking for comparisons

2. Implement logical opcodes:
   - `AND`, `OR` - Logical operations
   - `NOT` - Logical negation

3. Implement control flow opcodes:
   - `JUMP` - Unconditional jump
   - `JUMP_IF_FALSE` - Conditional jump
   - `JUMP_IF_TRUE` - Conditional jump (if needed)
   - IP manipulation for jumps

4. Unary operators:
   - `NEGATE` - Numeric negation
   - `POSITIVE` - Unary plus (identity)

**Example execution:**
```python
# LUCID:
if x > 5 { 10 } else { 20 }

# Bytecode:
LOAD_LOCAL x
CONSTANT 5
GT                  # Stack: [x > 5]
JUMP_IF_FALSE @else # Jump if false
CONSTANT 10         # Then branch
JUMP @end
@else:
CONSTANT 20         # Else branch
@end:
```

**Tests:**
- Comparison operators on Int and Float
- Logical operators (and, or, not)
- If expressions with both branches
- If expressions with only then branch
- Nested if expressions

**Success Criteria:**
```cpp
TEST_CASE("VM: If expressions") {
    // Compile and execute: if 5 > 3 { 100 } else { 200 }
    // Verify result is 100
}
```

---

### Day 4: Collections (Lists & Tuples)
**Goal**: Implement list and tuple operations

**Tasks:**
1. Implement collection construction:
   - `BUILD_LIST` - Build list from N stack items
     - Pop N items from stack
     - Create List value
     - Push list onto stack
   - `BUILD_TUPLE` - Build tuple from N stack items
     - Similar to BUILD_LIST
     - Create Tuple value

2. Implement indexing:
   - `INDEX` - Index into list/tuple
     - Pop index
     - Pop collection
     - Bounds checking
     - Push element onto stack
   - Runtime error for out-of-bounds

3. Collection operations:
   - Empty lists: `[]`
   - Nested collections: `[[1, 2], [3, 4]]`
   - Mixed-type tuples: `(1, "hello", true)`

**Example execution:**
```python
# LUCID:
let nums = [1, 2, 3]
let first = nums[0]

# Bytecode:
CONSTANT 1
CONSTANT 2
CONSTANT 3
BUILD_LIST 3        # Stack: [[1,2,3]]
STORE_LOCAL 0       # nums
LOAD_LOCAL 0        # nums
CONSTANT 0          # index
INDEX               # Stack: [1]
STORE_LOCAL 1       # first
```

**Tests:**
- List construction
- Tuple construction
- Indexing lists
- Indexing tuples
- Nested collections
- Out-of-bounds errors

**Success Criteria:**
```cpp
TEST_CASE("VM: Lists and tuples") {
    // Compile: let nums = [1, 2, 3]; nums[1]
    // Verify result is 2
}
```

---

### Day 5: Let Bindings & Tuple Destructuring
**Goal**: Variable bindings and pattern matching

**Tasks:**
1. Let bindings with simple variables:
   - Already works via `STORE_LOCAL`
   - Test with multiple let statements
   - Variable shadowing (same name in nested scopes)

2. Tuple destructuring:
   - `let (a, b) = tuple`
   - Compile tuple, then multiple STORE_LOCAL
   - Verify correct unpacking

3. Multiple bindings with dependencies:
   - `let x = 5; let y = x + 3; let z = y * 2`
   - Proper variable lifetimes
   - Correct local indices

4. Scope management:
   - Variables scoped to function
   - No block-level scoping yet (LUCID v0.3)

**Example execution:**
```python
# LUCID:
let (a, b) = (10, 20)
a + b

# Bytecode:
CONSTANT 10
CONSTANT 20
BUILD_TUPLE 2       # Stack: [(10, 20)]
DUP                 # Duplicate for destructuring
CONSTANT 0
INDEX               # Stack: [(10, 20), 10]
STORE_LOCAL 0       # a = 10
CONSTANT 1
INDEX               # Stack: [20]
STORE_LOCAL 1       # b = 20
LOAD_LOCAL 0
LOAD_LOCAL 1
ADD                 # Stack: [30]
```

**Tests:**
- Simple let bindings
- Multiple let bindings
- Tuple destructuring
- Complex destructuring patterns

**Success Criteria:**
```cpp
TEST_CASE("VM: Let bindings") {
    // Compile: let x = 5; let y = x + 3; y
    // Verify result is 8
}
```

---

### Day 6: Built-in Methods & Integration
**Goal**: Implement built-in methods and end-to-end testing

**Tasks:**
1. Implement `CALL_METHOD` opcode:
   - Read method name from constant pool
   - Pop receiver from stack
   - Dispatch to built-in method
   - Push result onto stack

2. Built-in methods:
   - **List.length()** → Int
   - **List.append(x)** → List (new list with x appended)
   - **String.length()** → Int
   - **Tuple.length()** → Int

3. Method dispatch table:
   - Map (Type, MethodName) → function pointer
   - Type checking for methods
   - Runtime errors for undefined methods

4. Integration testing:
   - Compile and execute all example programs
   - Fibonacci (recursive function calls)
   - Arithmetic (operators and let bindings)
   - Lists (construction, indexing, methods)

5. Error handling:
   - Stack overflow (too many recursive calls)
   - Division by zero
   - Type errors
   - Undefined variables
   - Out-of-bounds indexing

**Example execution:**
```python
# LUCID:
let nums = [1, 2, 3]
nums.length()

# Bytecode:
CONSTANT 1
CONSTANT 2
CONSTANT 3
BUILD_LIST 3        # Stack: [[1,2,3]]
STORE_LOCAL 0       # nums
LOAD_LOCAL 0        # nums
CALL_METHOD "length"  # Pop list, push Int(3)
```

**Tests:**
- Built-in methods on all types
- Method chaining (if supported)
- Complete program execution
- Error conditions

**Success Criteria:**
```cpp
TEST_CASE("VM: Built-in methods") {
    // Compile: [1, 2, 3].length()
    // Verify result is 3
}

TEST_CASE("VM: Execute fibonacci") {
    // Compile fibonacci.lucid
    // Execute: fibonacci(10)
    // Verify result is 55
}
```

---

## VM Interface Design

### Public API

```cpp
class VM {
public:
    // Constructor
    explicit VM();

    // Load bytecode into VM
    void load(const Bytecode& bytecode);

    // Execute bytecode starting at main()
    auto execute() -> Value;

    // Execute specific function
    auto call_function(const std::string& name, std::vector<Value> args = {}) -> Value;

    // Reset VM state
    void reset();

    // Get execution statistics (optional, for debugging)
    struct Stats {
        size_t instructions_executed;
        size_t max_stack_depth;
        size_t max_call_depth;
    };
    auto get_stats() const -> Stats;

private:
    // Bytecode being executed
    const Bytecode* bytecode_;

    // Value stack
    std::vector<Value> stack_;

    // Call stack
    std::vector<CallFrame> frames_;

    // Instruction pointer
    size_t ip_;

    // Execution statistics
    Stats stats_;

    // Stack operations
    void push(Value value);
    auto pop() -> Value;
    auto peek(size_t distance = 0) -> Value&;

    // Frame management
    void push_frame(CallFrame frame);
    void pop_frame();
    auto current_frame() -> CallFrame&;

    // Instruction reading
    auto read_byte() -> uint8_t;
    auto read_uint16() -> uint16_t;

    // Instruction execution
    void execute_instruction();

    // Built-in methods
    auto call_builtin_method(const Value& receiver, const std::string& method_name) -> Value;

    // Error handling
    void runtime_error(const std::string& message);
};
```

### CallFrame Structure

```cpp
struct CallFrame {
    size_t function_index;      // Which function is executing
    size_t return_address;      // IP to return to
    size_t frame_start;         // Index in value stack where locals start
    size_t local_count;         // Number of local variables

    // Helper to get local variable
    auto get_local(size_t index, const std::vector<Value>& stack) const -> const Value& {
        return stack[frame_start + index];
    }

    auto set_local(size_t index, std::vector<Value>& stack, Value value) -> void {
        stack[frame_start + index] = std::move(value);
    }
};
```

---

## Bytecode Execution Examples

### Example 1: Simple Arithmetic
```python
function add(x: Int, y: Int) returns Int {
    return x + y
}
```

**Execution trace:**
```
Initial state:
  stack: [5, 3]  (arguments)
  frames: []
  ip: 0

CALL add:
  Create frame: {function_index=0, return_address=X, frame_start=0, local_count=2}
  stack: [5, 3]
  frames: [frame0]
  ip: function_start

LOAD_LOCAL 0:
  stack: [5, 3, 5]

LOAD_LOCAL 1:
  stack: [5, 3, 5, 3]

ADD:
  Pop 3, pop 5
  Push 8
  stack: [5, 3, 8]

RETURN:
  result = pop() = 8
  Pop frame
  stack: [8]
  ip: return_address

Final result: 8
```

### Example 2: Fibonacci
```python
function fibonacci(n: Int) returns Int {
    return if n <= 1 {
        n
    } else {
        fibonacci(n - 1) + fibonacci(n - 2)
    }
}
```

**Execution trace (fibonacci(3)):**
```
Call fibonacci(3):
  3 <= 1? false
  Call fibonacci(2):
    2 <= 1? false
    Call fibonacci(1):
      1 <= 1? true
      return 1
    Call fibonacci(0):
      0 <= 1? true
      return 0
    return 1 + 0 = 1
  Call fibonacci(1):
    1 <= 1? true
    return 1
  return 1 + 1 = 2

Final result: 2
```

### Example 3: List Operations
```python
let nums = [1, 2, 3]
let first = nums[0]
nums.length()
```

**Execution trace:**
```
CONSTANT 1:
  stack: [1]

CONSTANT 2:
  stack: [1, 2]

CONSTANT 3:
  stack: [1, 2, 3]

BUILD_LIST 3:
  Pop 3, 2, 1
  Create List([1, 2, 3])
  stack: [List([1,2,3])]

STORE_LOCAL 0:  (nums)
  stack: [List([1,2,3])]

LOAD_LOCAL 0:
  stack: [List([1,2,3]), List([1,2,3])]

CONSTANT 0:
  stack: [List([1,2,3]), List([1,2,3]), 0]

INDEX:
  Pop 0, pop List
  Get element at index 0 → 1
  stack: [List([1,2,3]), 1]

STORE_LOCAL 1:  (first)
  stack: [List([1,2,3]), 1]

LOAD_LOCAL 0:
  stack: [List([1,2,3]), 1, List([1,2,3])]

CALL_METHOD "length":
  Pop List
  Call list.length() → 3
  stack: [List([1,2,3]), 1, 3]

Final result: 3
```

---

## Error Handling Strategy

### Runtime Errors

```cpp
class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& message, size_t ip)
        : std::runtime_error(message), ip_(ip) {}

    size_t ip() const { return ip_; }

private:
    size_t ip_;
};
```

### Error Types

1. **Type errors**
   - "Cannot add String and Int"
   - "Cannot index into Int"
   - "Cannot compare List and Int"

2. **Bounds errors**
   - "Index out of bounds: 5 >= 3"
   - "Negative index: -1"

3. **Call errors**
   - "Undefined function: foo"
   - "Wrong number of arguments"

4. **Stack errors**
   - "Stack overflow (max depth 1000)"
   - "Stack underflow (internal error)"

5. **Division errors**
   - "Division by zero"
   - "Modulo by zero"

### Error Reporting

```cpp
void VM::runtime_error(const std::string& message) {
    // Build error message with stack trace
    std::string error = fmt::format("Runtime error at IP {}: {}\n", ip_, message);

    // Add call stack
    error += "Call stack:\n";
    for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
        const auto& frame = *it;
        const auto& fn_name = bytecode_->function_names[frame.function_index];
        error += fmt::format("  in function '{}'\n", fn_name);
    }

    throw RuntimeError(error, ip_);
}
```

---

## Testing Strategy

### Unit Tests (per day)
- Day 1: Stack operations, basic arithmetic
- Day 2: Variable access, function calls
- Day 3: Comparisons, control flow
- Day 4: Collections, indexing
- Day 5: Let bindings, destructuring
- Day 6: Built-in methods, integration

### Integration Tests

```cpp
TEST_CASE("VM: Execute complete programs") {
    SECTION("Arithmetic") {
        auto source = read_file("examples/arithmetic.lucid");
        auto tokens = Lexer(source).tokenize();
        auto ast = Parser(tokens).parse();
        TypeChecker tc;
        tc.check(ast);
        Compiler compiler;
        auto bytecode = compiler.compile(ast);

        VM vm;
        vm.load(bytecode);
        auto result = vm.call_function("main");

        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 200);  // Expected result
    }

    SECTION("Fibonacci") {
        // Similar test for fibonacci
        auto result = vm.call_function("fibonacci", {Value(10)});
        REQUIRE(result.as_int() == 55);
    }

    SECTION("Lists") {
        // Test list operations
        auto result = vm.call_function("main");
        REQUIRE(result.as_int() == 3);  // [1,2,3][0] + [1,2,3][1]
    }
}
```

### Performance Tests

```cpp
TEST_CASE("VM: Performance") {
    // Fibonacci(30) should complete in < 1 second
    auto start = std::chrono::high_resolution_clock::now();
    auto result = vm.call_function("fibonacci", {Value(30)});
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    REQUIRE(duration.count() < 1000);
    REQUIRE(result.as_int() == 832040);
}
```

### Error Tests

```cpp
TEST_CASE("VM: Runtime errors") {
    SECTION("Division by zero") {
        // Compile: 5 / 0
        REQUIRE_THROWS_AS(vm.execute(), RuntimeError);
    }

    SECTION("Out of bounds") {
        // Compile: [1, 2, 3][10]
        REQUIRE_THROWS_AS(vm.execute(), RuntimeError);
    }

    SECTION("Type error") {
        // Compile: "hello" + 5
        REQUIRE_THROWS_AS(vm.execute(), RuntimeError);
    }
}
```

---

## Performance Considerations

### 1. Stack Allocation
- Pre-allocate value stack (e.g., 1024 elements)
- Grow dynamically only if needed
- Use `std::vector::reserve()` for known sizes

### 2. Instruction Dispatch
- Start with switch statement (simple, fast enough)
- Profile hot paths
- Consider computed goto optimization later

### 3. Value Copying
- Use move semantics where possible
- Avoid unnecessary copies in push/pop
- Consider value pooling for strings/lists (later optimization)

### 4. Call Frame Management
- Pre-allocate call stack (e.g., 256 frames)
- Catch stack overflow early

### 5. Built-in Methods
- Use function pointers or std::function
- Minimize virtual dispatch
- Inline simple methods (like length)

---

## Integration with Existing Code

### From Compiler (Phase 4)
```cpp
// After compilation
Compiler compiler;
auto bytecode = compiler.compile(program);

// Execute with VM
VM vm;
vm.load(bytecode);
auto result = vm.call_function("main");
```

### Complete Pipeline
```cpp
// Full pipeline: source → result
auto source = read_file("program.lucid");

// Lexer
Lexer lexer(source);
auto tokens = lexer.tokenize();

// Parser
Parser parser(tokens);
auto ast = parser.parse();

// Type checker
TypeChecker type_checker;
auto errors = type_checker.check(ast);
if (!errors.empty()) {
    // Report errors
    return;
}

// Compiler
Compiler compiler;
auto bytecode = compiler.compile(ast);

// VM
VM vm;
vm.load(bytecode);
auto result = vm.execute();

// Print result
std::cout << "Result: " << result << std::endl;
```

---

## Critical Files Summary

### Files to Create (2 new files):
1. `include/lucid/backend/vm.hpp` - VM interface (~200 lines)
2. `tests/vm_test.cpp` - VM test suite (~600 lines)

### Files to Implement (1 file):
1. `src/backend/vm.cpp` - VM implementation (~800-1200 lines)

### Files to Modify (1):
1. `CMakeLists.txt` - Add vm_test.cpp to tests

---

## Success Criteria

Phase 5 is complete when:
- ✅ VM executes all Phase 4 bytecode correctly
- ✅ All opcodes implemented (34 instructions)
- ✅ Stack operations working correctly
- ✅ Function calls and returns working
- ✅ Control flow (if expressions) working
- ✅ Collections (lists, tuples) working
- ✅ Indexing working with bounds checking
- ✅ Let bindings working
- ✅ Tuple destructuring working
- ✅ Built-in methods working (length, append)
- ✅ All example programs execute correctly:
  - arithmetic.lucid → correct result
  - fibonacci.lucid → fibonacci(10) = 55
  - lists.lucid → correct list operations
- ✅ Comprehensive test suite passes (~150+ test cases)
- ✅ Runtime error handling works correctly
- ✅ Clean integration with Phase 4 compiler
- ✅ Ready for Phase 6 (Standard Library & IO)

---

## Out of Scope (Deferred)

### Phase 6: Standard Library & IO
- print/println functions
- File I/O operations
- String operations beyond basic
- Advanced list operations

### Future Phases:
- Lambdas and closures (requires closure capture)
- Effect system (IO, Result, Option)
- Garbage collection (for cycles)
- Optimizations (bytecode optimization, JIT)

---

## Implementation Sequence

1. ✅ **Phase 4**: Bytecode compiler → COMPLETE (207 tests passing)
2. ✅ **Day 1**: VM infrastructure & arithmetic → COMPLETE
   - Implemented: RETURN, LOAD_LOCAL, STORE_LOCAL
   - All arithmetic operations working
   - All comparison operations working
   - All logical operations working
   - All unary operations working
3. ✅ **Day 2**: Variables & functions → COMPLETE (CALL, RETURN)
   - Simple function calls
   - Nested function calls
   - Recursive functions (factorial, fibonacci)
4. ✅ **Day 3**: Comparisons & control → COMPLETE (JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE)
   - If expressions with both branches
   - Nested if expressions
   - Complex boolean conditions
5. ✅ **Day 4**: Collections → COMPLETE (BUILD_LIST, BUILD_TUPLE, INDEX)
   - List construction and indexing
   - Tuple construction and indexing
   - Bounds checking with error handling
6. ✅ **Day 5**: Let bindings → COMPLETE (STORE_LOCAL, LOAD_LOCAL)
   - Simple let bindings
   - Multiple let bindings
   - Let with expressions and function calls
7. ✅ **Day 6**: Built-ins & polish → COMPLETE (CALL_METHOD)
   - List.length(), List.append(), List.head(), List.tail(), List.is_empty()
   - Tuple.length()
   - String.length(), String.is_empty()
   - Method chaining support

**Total**: ~700 lines in vm.cpp, ~350 lines of VM tests
**Progress**: 6/6 days complete (100%), **956 total assertions passing**

---

## Notes

- Follow existing C++20 patterns from Phase 4
- Use value.hpp Value class from Phase 4 (already has all operations)
- Extensive error checking and helpful error messages
- Test-driven development: write tests first, then implement
- Commit after each day's work
- Keep VM simple and readable (optimization comes later)
- Focus on correctness first, performance second

---

## Daily Checklist Template

Each day should follow this structure:

**Morning:**
1. Read the day's goals and tasks
2. Review relevant bytecode opcodes
3. Write test cases first (TDD)

**Implementation:**
1. Implement opcodes for the day
2. Run tests frequently
3. Debug failures with disassembler

**Evening:**
1. Ensure all tests pass
2. Test with example programs
3. Document any issues/decisions
4. Commit the day's work

---

## Example Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include "lucid/backend/vm.hpp"
#include "lucid/backend/compiler.hpp"
#include "lucid/frontend/parser.hpp"
#include "lucid/frontend/lexer.hpp"

using namespace lucid;

// Helper to compile and execute
auto compile_and_execute(const std::string& source) -> Value {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    TypeChecker tc;
    tc.check(ast);

    Compiler compiler;
    auto bytecode = compiler.compile(ast);

    VM vm;
    vm.load(bytecode);
    return vm.execute();
}

TEST_CASE("VM: Basic arithmetic", "[vm]") {
    SECTION("Addition") {
        auto result = compile_and_execute(R"(
            function main() returns Int {
                return 5 + 3
            }
        )");
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 8);
    }

    SECTION("Complex expression") {
        auto result = compile_and_execute(R"(
            function main() returns Int {
                return 2 + 3 * 4
            }
        )");
        REQUIRE(result.as_int() == 14);
    }
}

TEST_CASE("VM: Function calls", "[vm]") {
    auto result = compile_and_execute(R"(
        function add(x: Int, y: Int) returns Int {
            return x + y
        }

        function main() returns Int {
            return add(5, 3)
        }
    )");
    REQUIRE(result.as_int() == 8);
}

// ... more tests
```

---

## Completion Log

### Day 1 - January 4, 2026 ✅

**Goal**: VM infrastructure and basic arithmetic execution

**Completed deliverables:**
- ✅ VM class with execution loop
- ✅ Stack operations (push, pop, peek)
- ✅ Instruction reading (read_byte, read_uint16)
- ✅ All arithmetic opcodes (ADD, SUB, MUL, DIV, MOD, POW)
- ✅ All comparison opcodes (EQ, NE, LT, GT, LE, GE)
- ✅ All logical opcodes (AND, OR, NOT)
- ✅ All unary opcodes (NEGATE, POSITIVE)
- ✅ Basic variable opcodes (LOAD_LOCAL, STORE_LOCAL)
- ✅ RETURN opcode for function returns
- ✅ TRUE, FALSE, CONSTANT opcodes
- ✅ Type coercion (Int + Float → Float)
- ✅ Division by zero error handling

**Test Results**: **67 assertions in 21 test cases - ALL PASSING**

**Key Achievements:**
- Complete arithmetic evaluation
- Comparison and logical operations
- Proper type checking and coercion
- Error handling for division by zero
- Fixed test helper to support Bool return types

**Total Project Status**: 878 assertions passing across all phases!

---

### Day 2-6 - January 31, 2026 ✅

**Goal**: Complete all remaining VM functionality

**Completed deliverables:**
- ✅ Function calls (CALL opcode) with argument passing
- ✅ Recursive functions (factorial, fibonacci working)
- ✅ Control flow (JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE)
- ✅ Collections (BUILD_LIST, BUILD_TUPLE, INDEX)
- ✅ Let bindings with expressions
- ✅ Built-in methods (CALL_METHOD opcode):
  - List: length(), append(), head(), tail(), is_empty()
  - Tuple: length()
  - String: length(), is_empty()
- ✅ Method chaining support
- ✅ Updated type checker for new methods

**Test Results**: **145 assertions in 57 VM test cases - ALL PASSING**

**Key Achievements:**
- Complete VM execution of LUCID programs
- Full recursion support (fibonacci(15) = 610 verified)
- Collection operations with proper immutability (append returns new list)
- Integration tests: recursive sum over list elements

**Total Project Status**: 956 assertions passing across all phases!

---

## Next Steps After Phase 5

Once Phase 5 is complete:
1. Update README.md to reflect completion
2. Create PHASE6_STDLIB_PLAN.md
3. Begin implementing standard library
4. Add IO operations (print, println, read)
5. Build more complex example programs

---

**Ready to begin Phase 5!**
