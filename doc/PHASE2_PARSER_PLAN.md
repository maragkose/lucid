# LUCID Compiler - Phase 2: Parser Implementation Plan

## Overview
Implement a recursive descent parser with operator precedence climbing for the LUCID programming language. The parser should transform tokens from the lexer into an Abstract Syntax Tree (AST).

## Context
- **Phase**: Phase 2 - Parser (AST Generation)
- **Dependencies**: Phase 1 Lexer (✓ Complete)
- **Language**: C++20
- **Target**: Linux
- **Performance**: Single-pass parsing with error recovery

## Phase 2 Goals
1. ✅ Define complete AST node structures
2. ✅ Implement expression parsing with operator precedence
3. ✅ Implement statement parsing
4. ✅ Implement function and program parsing
5. ✅ Add comprehensive error handling and recovery
6. ⏳ Complete integration tests
7. ⏳ Add AST visualization/printing utilities

## Implementation Plan: 5-Day Breakdown

### Day 1: AST Structures & Parser Foundation
**Goal**: Set up the AST node hierarchy and parser skeleton

#### Tasks:
- [x] Define AST node base classes (Expr, Stmt, Decl)
- [x] Define expression node types:
  - Literals (Int, Float, String, Bool)
  - Identifier
  - Binary and Unary operations
  - Function calls, method calls, indexing
  - Tuple, List, Lambda, If, Block expressions
- [x] Define statement node types:
  - Let statement (with pattern matching)
  - Return statement
  - Expression statement
- [x] Define type AST nodes:
  - Named types (Int, Float, String, Bool)
  - List types (List[T])
  - Tuple types ((T1, T2, ...))
- [x] Define Pattern nodes (for destructuring):
  - Identifier pattern
  - Tuple pattern
- [x] Define top-level nodes:
  - Function definition
  - Parameter
  - Program (collection of functions)
- [x] Create parser class skeleton with:
  - Token stream management (peek, advance, check, match, expect)
  - Error handling (error recording, synchronization)
  - Source location tracking

**Files**:
- `include/ast.h` - AST node definitions
- `include/parser.h` - Parser class interface
- `src/frontend/parser.cpp` - Initial skeleton

**Deliverables**:
- Complete AST hierarchy with proper inheritance
- Parser class with token management methods
- Error reporting infrastructure

---

### Day 2: Expression Parsing - Literals & Primary
**Goal**: Parse simple expressions and primary expressions

#### Tasks:
- [x] Implement `parse_expression()` - entry point
- [x] Implement `parse_primary()` for:
  - Integer literals
  - Float literals
  - String literals
  - Boolean literals (true/false)
  - Identifiers
  - Parenthesized expressions
  - Empty tuples: `()`
- [x] Implement `parse_tuple_or_grouped()`:
  - Grouped expression: `(expr)`
  - Tuple: `(expr1, expr2, ...)` with trailing comma support
- [x] Implement `parse_list_literal()`:
  - Empty list: `[]`
  - List with elements: `[1, 2, 3]` with trailing comma support
- [x] Write comprehensive tests for all literal types
- [x] Test tuple vs grouped expression distinction

**Test Cases**:
```cpp
TEST_CASE("Parser: Integer literals")
TEST_CASE("Parser: Float literals")
TEST_CASE("Parser: String literals")
TEST_CASE("Parser: Boolean literals")
TEST_CASE("Parser: Identifiers")
TEST_CASE("Parser: Empty tuple")
TEST_CASE("Parser: Tuple with elements")
TEST_CASE("Parser: Grouped expression vs tuple")
TEST_CASE("Parser: List literals")
```

---

### Day 3: Expression Parsing - Operators & Precedence
**Goal**: Implement operator precedence climbing for binary and unary operators

#### Tasks:
- [x] Define operator precedence levels:
  - LOWEST (0)
  - OR (10)
  - AND (20)
  - COMPARISON (30) - ==, !=, <, >, <=, >=
  - ADDITIVE (40) - +, -
  - MULTIPLICATIVE (50) - *, /, %
  - POWER (60) - **
  - UNARY (70) - not, -, +
  - POSTFIX (80) - calls, indexing
- [x] Implement `parse_precedence(min_prec)` - operator precedence climbing
- [x] Implement `parse_prefix()` for unary operators:
  - `not expr`
  - `-expr`
  - `+expr`
- [x] Handle binary operators with proper precedence
- [x] Handle right associativity for `**` (power)
- [x] Implement `parse_postfix()` for:
  - Function calls: `f(x, y)`
  - Method calls: `obj.method(args)`
  - Index access: `arr[index]`
  - Chained operations: `f(x)(y)`, `arr[i][j]`
- [x] Write comprehensive operator tests

**Test Cases**:
```cpp
TEST_CASE("Parser: Simple binary operators")
TEST_CASE("Parser: All arithmetic operators")
TEST_CASE("Parser: Comparison operators")
TEST_CASE("Parser: Logical operators")
TEST_CASE("Parser: Multiplication before addition")
TEST_CASE("Parser: Power before multiplication")
TEST_CASE("Parser: Comparison before logical and")
TEST_CASE("Parser: And before or")
TEST_CASE("Parser: Right associativity of power")
TEST_CASE("Parser: Unary operators")
TEST_CASE("Parser: Function calls")
TEST_CASE("Parser: Method calls")
TEST_CASE("Parser: Index access")
```

---

### Day 4: Complex Expressions - Lambda, If, Block
**Goal**: Parse complex expression forms

#### Tasks:
- [x] Implement `parse_lambda_expression()`:
  - No parameters: `lambda: expr`
  - Single parameter: `lambda x: expr`
  - Multiple parameters: `lambda x, y, z: expr`
  - Block body: `lambda x: { statements }`
  - Expression body: `lambda x: x + 1`
- [x] Implement `parse_if_expression()`:
  - If without else: `if cond { body }`
  - If with else: `if cond { then } else { else_body }`
  - Else if chains: `if c1 { b1 } else if c2 { b2 } else { b3 }`
- [x] Implement `parse_block_expression()`:
  - Parse statements within `{ ... }`
  - Handle empty blocks
  - Support let, return, and expression statements
- [x] Write tests for all complex expression forms
- [x] Test nested lambdas
- [x] Test nested if expressions

**Test Cases**:
```cpp
TEST_CASE("Parser: Lambda no params")
TEST_CASE("Parser: Lambda with params")
TEST_CASE("Parser: Lambda with block body")
TEST_CASE("Parser: Nested lambdas")
TEST_CASE("Parser: If without else")
TEST_CASE("Parser: If with else")
TEST_CASE("Parser: If else if chains")
TEST_CASE("Parser: Block expressions")
TEST_CASE("Parser: Lambda in list")
TEST_CASE("Parser: Function call with lambda")
```

---

### Day 5: Statements, Functions & Programs (CURRENT)
**Goal**: Parse top-level constructs and complete the parser

#### Tasks:
- [x] Implement `parse_statement()`:
  - Dispatch to specific statement parsers
- [x] Implement `parse_let_statement()`:
  - Simple binding: `let x = expr`
  - With type annotation: `let x: Int = expr`
  - Tuple destructuring: `let (x, y) = tuple`
  - Nested destructuring: `let ((a, b), c) = nested`
- [x] Implement `parse_return_statement()`:
  - `return expr`
- [x] Implement `parse_expression_statement()`:
  - Any expression as statement
- [x] Implement `parse_pattern()`:
  - Identifier pattern: `x`
  - Tuple pattern: `(x, y, z)`
  - Nested patterns: `((a, b), c)`
- [x] Implement `parse_type()`:
  - Named types: `Int`, `Float`, `String`, `Bool`
  - List types: `List[T]`
  - Tuple types: `(T1, T2, ...)`
- [x] Implement `parse_parameter()`:
  - `name: Type`
- [x] Implement `parse_function()`:
  - Function signature: `function name(params) returns RetType`
  - Function body (block)
- [x] Implement `parse_program()`:
  - Parse all top-level functions
  - Handle errors and synchronization
- [x] Write comprehensive integration tests

**Test Cases**:
```cpp
TEST_CASE("Parser: Let statements")
TEST_CASE("Parser: Let with tuple destructuring")
TEST_CASE("Parser: Let with type annotation")
TEST_CASE("Parser: Let with List type")
TEST_CASE("Parser: Let with Tuple type")
TEST_CASE("Parser: Simple function")
TEST_CASE("Parser: Function with parameters")
TEST_CASE("Parser: Function with complex types")
TEST_CASE("Parser: Multiple functions in program")
TEST_CASE("Parser: Complete program with all features")
TEST_CASE("Parser: Nested destructuring patterns")
```

---

## Current Implementation Status

### ✅ Completed
- All AST node definitions
- Token stream management
- Error handling and synchronization
- Complete expression parsing (literals, operators, calls, indexing)
- Operator precedence with proper associativity
- Complex expressions (lambda, if, block, tuple, list)
- Statement parsing (let, return, expression)
- Pattern matching for destructuring
- Type parsing (named, list, tuple)
- Function and program parsing
- Comprehensive test suite (~800 lines)

### ⏳ In Progress / Next Steps
1. **Error Recovery Improvements** (Day 5-6):
   - Test error synchronization
   - Add more error cases
   - Improve error messages

2. **AST Utilities** (Day 6):
   - Implement AST visitor pattern
   - Add AST printing for debugging
   - Add AST validation pass

3. **Integration Testing** (Day 6):
   - Test full LUCID programs from examples/
   - Ensure all features work together
   - Performance testing

## File Structure

```
include/
  ├── ast.h                    # ✅ AST node definitions (~500 lines)
  └── parser.h                 # ✅ Parser interface (~150 lines)

src/frontend/
  └── parser.cpp               # ✅ Parser implementation (~860 lines)

tests/
  └── parser_test.cpp          # ✅ Comprehensive tests (~811 lines)
```

## Key Implementation Decisions

### 1. Operator Precedence
Using **precedence climbing algorithm** instead of separate precedence methods:
- More maintainable
- Easier to adjust precedence levels
- Handles associativity cleanly

### 2. Error Handling
**Panic mode recovery**:
- Record errors but continue parsing
- Synchronize at statement boundaries
- Return nullptr on error

### 3. Expressions vs Statements
LUCID treats many constructs as expressions:
- If expressions can be values
- Blocks are expressions
- Lambda expressions are first-class

### 4. Pattern Matching
Support destructuring in let statements:
```lucid
let (x, y) = (1, 2)
let ((a, b), c) = ((1, 2), 3)
```

## Testing Strategy

### Unit Tests (✅ Complete)
- Test each parsing method independently
- Test operator precedence
- Test error cases
- Total: ~50 test cases

### Integration Tests (⏳ Next)
- Parse complete example programs
- Test all features working together
- Performance benchmarks

### Example Programs to Parse
```lucid
# examples/fibonacci.lucid
function fibonacci(n: Int) returns Int {
    if n <= 1 {
        return n
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2)
    }
}

function main() returns Int {
    let result = fibonacci(10)
    return result
}
```

## Performance Targets

- **Parsing Speed**: Process 10,000 lines/second
- **Memory**: Minimize AST node allocations
- **Error Recovery**: Continue after first error
- **Single Pass**: No backtracking

## Next Phase Preview: Phase 3 - Type Checker

After completing Phase 2, Phase 3 will implement:
1. Type inference
2. Type checking all expressions and statements
3. Symbol table construction
4. Function signature validation
5. Type error reporting

## Success Criteria

Phase 2 is complete when:
- [x] All expression types parse correctly
- [x] All statement types parse correctly
- [x] Functions and programs parse correctly
- [x] Operator precedence is correct
- [x] Pattern matching works
- [x] Type annotations parse correctly
- [ ] All unit tests pass (currently passing)
- [ ] Integration tests with example programs pass
- [ ] Error recovery works properly
- [ ] AST can be printed for debugging
- [ ] No memory leaks (verified with sanitizers)

## Commands for Development

```bash
# Build the project
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Run parser tests
./lucid-tests "[parser]"

# Run all tests
./lucid-tests

# Run with verbose output
./lucid-tests "[parser]" -s

# Check for memory leaks (sanitizers enabled in Debug)
./lucid-tests
```

## Notes

- Parser is mostly complete (Day 5)
- Focus next on integration tests and utilities
- Error messages could be improved with more context
- Consider adding AST visitor pattern for Phase 3
- May need to refactor AST structure for semantic analysis
