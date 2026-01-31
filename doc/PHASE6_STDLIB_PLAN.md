# LUCID Compiler - Phase 6: Standard Library & IO

**Status**: ✅ COMPLETE

## Overview
Implement core standard library functions and IO operations to make LUCID programs practically useful. This phase adds print/println for output, and expands collection operations.

## Context
- **Input**: Working VM from Phase 5 (956 tests passing)
- **Output**: Executable programs with console output
- **Scope**: Console IO, expanded collection methods, string operations
- **Foundation**: Builds on Phase 5's VM and CALL_METHOD infrastructure

## Prerequisites (Complete from Phase 5)
✅ Stack-based VM executing bytecode
✅ Function calls and recursion
✅ Control flow (if expressions)
✅ Collections (List, Tuple) with basic methods
✅ CALL_METHOD opcode for built-in methods
✅ 956 tests passing

## Goals

### Primary Goals
1. **Console Output**: `print()` and `println()` functions
2. **String Operations**: Expanded string methods
3. **Collection Methods**: map, filter, fold for lists
4. **Numeric Conversions**: Int/Float to String conversions

### Secondary Goals (if time permits)
- Console input (`read_line()`)
- File IO basics
- More string methods (split, join, trim)

---

## Design Decisions

### 1. IO Model

For simplicity, we'll implement IO as **immediate side effects** rather than the full `IO[T]` monad from the spec. This is a pragmatic choice for the initial implementation.

```python
# Simplified: print has immediate effect
print("Hello")  # Prints immediately, returns Unit

# Full spec would be:
# print("Hello")  # Returns IO[Unit], needs await to execute
```

**Rationale**: The full IO monad requires more infrastructure (effect tracking, await handling). We'll add that in a future phase.

### 2. Built-in Functions vs Methods

Two categories of built-ins:

**A. Global Functions** (new):
- `print(value)` - Print without newline
- `println(value)` - Print with newline
- `to_string(value)` - Convert any value to string

**B. Methods on Types** (expand existing):
- String: `to_upper()`, `to_lower()`, `trim()`, `split()`, `contains()`
- List: `map()`, `filter()`, `fold()`, `reverse()`, `concat()`
- Int/Float: `to_string()`

### 3. Implementation Strategy

**Option A**: New opcode for global builtins (CALL_BUILTIN)
**Option B**: Treat builtins as special functions in the function table

We'll use **Option A** - add a `CALL_BUILTIN` opcode:
- Cleaner separation of user functions vs builtins
- Easier to extend later
- No function table pollution

---

## Implementation Plan: 4-Day Breakdown

### Day 1: Print/Println and Basic Output

**Tasks:**
1. Add `CALL_BUILTIN` opcode to bytecode.hpp
2. Implement print/println in VM
3. Update compiler to recognize builtin functions
4. Update type checker for print/println signatures
5. Add to_string() for all Value types

**Bytecode Changes:**
```cpp
enum class OpCode : uint8_t {
    // ... existing opcodes ...
    CALL_BUILTIN,    // Call built-in function [builtin_id: uint16_t, arg_count: uint8_t]
};
```

**Built-in IDs:**
```cpp
enum class BuiltinId : uint16_t {
    PRINT = 0,
    PRINTLN = 1,
    TO_STRING = 2,
};
```

**Type Signatures:**
```
print(value: Any) -> Unit
println(value: Any) -> Unit
to_string(value: Any) -> String
```

**Tests:**
- Print integer, float, bool, string
- Println with various types
- to_string conversions

---

### Day 2: String Methods

**Tasks:**
1. Add string methods to type checker
2. Implement in VM's call_builtin_method

**Methods to Add:**
```python
# Query methods
String.contains(substring: String) -> Bool
String.starts_with(prefix: String) -> Bool
String.ends_with(suffix: String) -> Bool

# Transformation methods
String.to_upper() -> String
String.to_lower() -> String
String.trim() -> String

# Splitting/joining
String.split(delimiter: String) -> List[String]
```

**Tests:**
- contains, starts_with, ends_with
- Case transformations
- Trim whitespace
- Split strings

---

### Day 3: List Higher-Order Methods

**Tasks:**
1. Implement map, filter, fold for lists
2. Add reverse, concat methods
3. Handle function values as arguments

**Challenge**: map/filter/fold take function arguments. Need to:
- Support function values (lambdas or function references)
- Call user functions from within builtin methods

**Methods to Add:**
```python
List[T].map(f: T -> U) -> List[U]
List[T].filter(pred: T -> Bool) -> List[T]
List[T].fold(init: U, f: (U, T) -> U) -> U
List[T].reverse() -> List[T]
List[T].concat(other: List[T]) -> List[T]
```

**Implementation Notes:**
- For now, implement simpler versions without lambdas
- `reverse()` and `concat()` don't need function args
- map/filter/fold may be deferred if lambda support is complex

**Tests:**
- reverse([1,2,3]) == [3,2,1]
- [1,2].concat([3,4]) == [1,2,3,4]

---

### Day 4: Numeric Methods and Integration

**Tasks:**
1. Add Int/Float methods
2. Integration testing with full programs
3. Documentation and cleanup

**Methods to Add:**
```python
Int.to_string() -> String
Int.abs() -> Int
Float.to_string() -> String
Float.abs() -> Float
Float.floor() -> Int
Float.ceil() -> Int
Float.round() -> Int
```

**Integration Tests:**
- Print fibonacci sequence
- Print factorial results
- String manipulation programs

---

## File Changes

### Files to Modify

**include/lucid/backend/bytecode.hpp:**
- Add CALL_BUILTIN opcode
- Add BuiltinId enum

**src/backend/bytecode.cpp:**
- Add opcode_name for CALL_BUILTIN
- Add disassembly support

**src/backend/compiler.cpp:**
- Recognize builtin function calls
- Emit CALL_BUILTIN instead of CALL

**src/backend/vm.cpp:**
- Handle CALL_BUILTIN opcode
- Implement print, println, to_string
- Expand call_builtin_method for new methods

**src/semantic/type_checker.cpp:**
- Add builtin function signatures
- Add new method signatures

**tests/vm_test.cpp:**
- Add tests for all new functionality

---

## Detailed Specifications

### Print/Println Behavior

```cpp
// print: output without newline
print(42)        // Output: 42
print("hello")   // Output: hello
print(true)      // Output: true
print([1,2,3])   // Output: [1, 2, 3]

// println: output with newline
println(42)      // Output: 42\n
println("hello") // Output: hello\n
```

### Value to String Conversion

```cpp
// Int
42.to_string()         // "42"
(-5).to_string()       // "-5"

// Float
3.14.to_string()       // "3.14"
(-2.5).to_string()     // "-2.5"

// Bool
true.to_string()       // "true"
false.to_string()      // "false"

// String
"hello".to_string()    // "hello"

// List
[1, 2, 3].to_string()  // "[1, 2, 3]"

// Tuple
(1, "a").to_string()   // "(1, \"a\")"
```

### String Methods

```cpp
// contains
"hello world".contains("world")  // true
"hello".contains("xyz")          // false

// starts_with / ends_with
"hello".starts_with("hel")       // true
"hello".ends_with("lo")          // true

// case conversion
"Hello".to_upper()               // "HELLO"
"Hello".to_lower()               // "hello"

// trim
"  hello  ".trim()               // "hello"

// split
"a,b,c".split(",")               // ["a", "b", "c"]
"hello".split("")                // ["h", "e", "l", "l", "o"]
```

---

## Testing Strategy

### Unit Tests
- Each builtin function
- Each new method
- Edge cases (empty strings, empty lists)

### Integration Tests
```python
# Fibonacci with output
function print_fib(n: Int) {
    let a = 0
    let b = 1
    println(a)
    println(b)
    # ... would need loops, so use recursion
}

# Greeting program
function greet(name: String) {
    println("Hello, " + name + "!")
}
```

### Output Capture
For testing print/println, we need to capture stdout:
- Use a test mode that captures output to a buffer
- Or redirect stdout in tests

---

## Success Criteria

Phase 6 is complete when:
- ✅ print() and println() work for all types
- ✅ to_string() works for all types
- ✅ String methods: contains, starts_with, ends_with, to_upper, to_lower, trim
- ✅ List methods: reverse, concat
- ✅ Numeric methods: abs, to_string (Int/Float)
- ✅ All tests passing
- ✅ Can write and run simple interactive programs

---

## Out of Scope (Future Phases)

- File IO (read_file, write_file)
- Network IO (http_get, http_post)
- Full IO monad with await
- Lambda expressions as first-class values
- map/filter/fold with function arguments

---

## Notes

- Keep implementation simple and pragmatic
- Focus on practical utility over spec completeness
- IO effects are immediate (not deferred via IO monad)
- Test thoroughly with real programs

---

## Completion Summary - January 31, 2026

### Day 1: Print/Println ✅
- Added `CALL_BUILTIN` opcode for global builtin functions
- Implemented `print(value)` - output without newline
- Implemented `println(value)` - output with newline
- Implemented `to_string(value)` - convert any value to string
- Added output capture for testing (VM output buffer)

### Day 2: String Methods ✅
- `String.contains(substr)` - substring search
- `String.starts_with(prefix)` - prefix matching
- `String.ends_with(suffix)` - suffix matching
- `String.to_upper()` - uppercase conversion
- `String.to_lower()` - lowercase conversion
- `String.trim()` - whitespace trimming

### Day 3: List Methods ✅
- `List.reverse()` - reverse list order
- `List.concat(other)` - concatenate lists
- (head, tail, append, is_empty, length already existed from Phase 5)

### Day 4: Numeric Methods ✅
- `Int.to_string()` - integer to string
- `Int.abs()` - absolute value
- `Float.to_string()` - float to string
- `Float.abs()` - absolute value
- `Float.floor()` - floor to integer
- `Float.ceil()` - ceiling to integer
- `Float.round()` - round to nearest integer

### Test Results
- **31 new Phase 6 test cases**
- **1003 total assertions** passing (up from 956)
- All functionality tested including edge cases

### Files Modified
- `include/lucid/backend/bytecode.hpp` - Added CALL_BUILTIN, BuiltinId
- `src/backend/bytecode.cpp` - Opcode support
- `src/backend/compiler.cpp` - Emit CALL_BUILTIN for builtins
- `src/backend/vm.cpp` - Implement builtins and new methods
- `include/lucid/backend/vm.hpp` - Output stream support
- `src/semantic/type_checker.cpp` - Type signatures for all new methods
- `tests/vm_test.cpp` - Comprehensive tests

**Phase 6 Complete!**
