# LUCID Compiler - Phase 3: Type Checker Implementation Plan

## Overview
Implement a semantic analyzer and type checker for the LUCID programming language. The type checker should validate types, perform type inference, build a symbol table, and report type errors.

## Context
- **Phase**: Phase 3 - Type Checker (Semantic Analysis)
- **Dependencies**:
  - Phase 1: Lexer (✓ Complete)
  - Phase 2: Parser & AST (✓ Complete)
- **Language**: C++20
- **Target**: Linux
- **Approach**: AST visitor pattern with symbol table

## Phase 3 Goals
1. Implement type representation and comparison
2. Build symbol tables for scopes
3. Implement type inference for local bindings
4. Type check all expressions
5. Type check all statements
6. Validate function signatures
7. Report clear type errors with source locations

## Language Subset (Phase 1/2 Features)

### Primitive Types
- `Int` - Arbitrary precision integer
- `Float` - 64-bit floating point
- `String` - UTF-8 strings
- `Bool` - Boolean values

### Composite Types
- `List[T]` - Homogeneous lists
- `(T1, T2, ...)` - Heterogeneous tuples

### Features to Type Check
- **Literals**: integers, floats, strings, booleans
- **Variables**: let bindings with optional type annotations
- **Functions**: typed parameters and return types
- **Operators**: arithmetic (+, -, *, /, %, **), comparison (==, !=, <, >, <=, >=), logical (and, or, not)
- **Control flow**: if expressions (must have compatible branches)
- **Collections**: lists, tuples, indexing
- **Lambdas**: anonymous functions with inferred types
- **Pattern matching**: tuple destructuring in let statements

## Implementation Plan: 6-Day Breakdown

### Day 1: Type System Infrastructure
**Goal**: Build the type representation and comparison system

#### Tasks:
1. Create type representation classes (`type_system.hpp/cpp`):
   - `SemanticType` base class
   - `PrimitiveType` (Int, Float, String, Bool)
   - `ListType` with element type
   - `TupleType` with element types vector
   - `FunctionType` with parameter and return types
   - `TypeVariable` for type inference
   - `UnknownType` for error recovery

2. Implement type operations:
   - Type equality checking
   - Type compatibility/unification
   - Type substitution (for generics future)
   - Pretty printing types for error messages

3. Create type environment/context:
   - Store builtin types (Int, Float, String, Bool)
   - Type lookup by name

**Files**:
- `include/lucid/semantic/type_system.hpp`
- `src/semantic/type_system.cpp`

**Tests**:
- Type equality tests
- Type compatibility tests
- Type printing tests

---

### Day 2: Symbol Table & Scoping
**Goal**: Implement symbol table with lexical scoping

#### Tasks:
1. Create `SymbolTable` class (`symbol_table.hpp/cpp`):
   - Support nested scopes (functions, blocks, lambdas)
   - Variable declarations with types
   - Function declarations with signatures
   - Scope enter/exit

2. Implement symbol lookup:
   - Resolve identifiers to declarations
   - Check for redeclaration errors
   - Handle shadowing correctly

3. Create `Symbol` class:
   - Variable symbols (name, type, location, mutable flag)
   - Function symbols (name, parameter types, return type, location)

**Files**:
- `include/lucid/semantic/symbol_table.hpp`
- `src/semantic/symbol_table.cpp`

**Tests**:
- Scope nesting tests
- Variable shadowing tests
- Undeclared variable detection

---

### Day 3: Type Checker Core (Expressions Part 1)
**Goal**: Type check literals, identifiers, and basic expressions

#### Tasks:
1. Create `TypeChecker` class (`type_checker.hpp/cpp`):
   - Visitor pattern for AST traversal
   - Symbol table management
   - Error collection

2. Implement expression type checking:
   - Literals (Int, Float, String, Bool) → infer type
   - Identifiers → lookup in symbol table
   - Binary operators:
     - Arithmetic (+, -, *, /, %, **) → Int/Float with type promotion
     - Comparison (==, !=) → any type → Bool
     - Ordering (<, >, <=, >=) → Int/Float → Bool
     - Logical (and, or) → Bool → Bool
   - Unary operators:
     - not → Bool → Bool
     - -, + → Int/Float → same type

3. Implement type coercion rules:
   - Int promotes to Float in mixed arithmetic
   - No implicit string conversions

**Files**:
- `include/lucid/semantic/type_checker.hpp`
- `src/semantic/type_checker.cpp`

**Tests**:
- Literal type inference
- Binary operator type checking
- Type error detection (e.g., "hello" + 5)

---

### Day 4: Type Checker (Expressions Part 2)
**Goal**: Type check complex expressions

#### Tasks:
1. Implement type checking for:
   - **Tuples**: (e1, e2, ...) → (T1, T2, ...)
   - **Lists**: [e1, e2, ...] → List[T] (all elements must be same type)
   - **Index access**: expr[index]
     - List[T][Int] → T
     - Tuple[T1, T2, ...][Int literal] → Ti
   - **Function calls**: f(args)
     - Check argument types match parameter types
     - Return function's return type
   - **Method calls**: obj.method(args)
     - Builtin methods on collections

2. Implement type checking for:
   - **If expressions**: if cond { then } else { else }
     - Condition must be Bool
     - Both branches must have compatible types
     - Result type is union of branch types
   - **Lambda expressions**: lambda x, y: body
     - Infer parameter types from usage
     - Infer return type from body

3. Implement type checking for:
   - **Block expressions**: { stmts }
     - Type is type of final expression or Unit

**Tests**:
- Tuple type inference
- List homogeneity checking
- Function call type checking
- If expression branch compatibility
- Lambda type inference

---

### Day 5: Statements & Functions
**Goal**: Type check statements and function definitions

#### Tasks:
1. Implement statement type checking:
   - **Let statements**: let x: T = expr
     - Check expr type matches declared type (if present)
     - Infer type if not annotated
     - Add to symbol table
     - Handle tuple destructuring: let (x, y) = tuple
   - **Return statements**: return expr
     - Check expr type matches function return type
     - Track if function has return on all paths
   - **Expression statements**: expr;
     - Type check expression, ignore result

2. Implement function type checking:
   - Check parameter types are valid
   - Check return type is valid
   - Type check function body
   - Verify all paths return correct type
   - Add function to global symbol table

3. Implement pattern type checking:
   - Identifier patterns → infer from initializer
   - Tuple patterns → destructure tuple type
   - Check pattern structure matches initializer type

**Tests**:
- Let statement type inference
- Let with type annotation checking
- Tuple destructuring type checking
- Function return type validation
- Missing return detection

---

### Day 6: Integration, Error Reporting & Completion
**Goal**: Complete type checker with excellent error messages

#### Tasks:
1. Improve error reporting:
   - Clear error messages with source locations
   - Type mismatch errors show expected vs actual
   - Helpful suggestions (e.g., did you mean to use Float?)
   - Multiple errors reported (don't stop at first)

2. Implement program-level type checking:
   - Check all function definitions
   - Verify main function exists with correct signature
   - Build complete symbol table
   - Report unused variables/functions (warnings)

3. Create type checker utilities:
   - Type pretty printer for diagnostics
   - Type error formatter
   - Integration with existing error infrastructure

4. Integration testing:
   - Type check all example programs from examples/
   - Ensure parser → type checker pipeline works
   - Add comprehensive integration tests

5. Error recovery:
   - Continue type checking after errors
   - Use Unknown type for error recovery
   - Avoid cascading errors

**Tests**:
- Integration tests with full programs
- Error message quality tests
- Multiple error reporting
- Error recovery tests

---

## File Structure

### New Files to Create

```
include/lucid/semantic/
  ├── type_system.hpp      # Type representation
  ├── symbol_table.hpp     # Symbol table & scopes
  └── type_checker.hpp     # Main type checker

src/semantic/
  ├── type_system.cpp      # Type operations
  ├── symbol_table.cpp     # Symbol management
  └── type_checker.cpp     # Type checking visitors

tests/
  └── type_checker_test.cpp  # Comprehensive tests
```

### Files to Modify

```
CMakeLists.txt              # Add semantic/*.cpp to build
include/lucid/frontend/ast.hpp  # Add type field to expressions (optional)
```

## Type Checking Algorithm

### Overall Approach
1. **Two-pass analysis**:
   - Pass 1: Collect all function signatures
   - Pass 2: Type check function bodies

2. **Bidirectional type checking**:
   - Some expressions infer types (literals, identifiers)
   - Some expressions check against expected type (function args)

3. **Type inference**:
   - Let bindings without annotations infer from initializer
   - Lambda parameters infer from usage context
   - Use Hindley-Milner-style unification (simplified)

### Type Checking Rules

#### Expressions
```
Γ ⊢ n : Int                                    (int literal)
Γ ⊢ f : Float                                  (float literal)
Γ ⊢ "s" : String                               (string literal)
Γ ⊢ true : Bool, Γ ⊢ false : Bool             (bool literal)

Γ(x) = T
─────────                                      (variable)
Γ ⊢ x : T

Γ ⊢ e1 : Int    Γ ⊢ e2 : Int
────────────────────────────                   (int arithmetic)
Γ ⊢ e1 op e2 : Int    where op ∈ {+,-,*,/,%,**}

Γ ⊢ e1 : T    Γ ⊢ e2 : T
────────────────────────                       (comparison)
Γ ⊢ e1 op e2 : Bool    where op ∈ {==,!=,<,>,<=,>=}

Γ ⊢ cond : Bool    Γ ⊢ then : T    Γ ⊢ else : T
──────────────────────────────────────────────  (if expression)
Γ ⊢ if cond { then } else { else } : T

Γ ⊢ e1 : T1    Γ ⊢ e2 : T2    ...
─────────────────────────────────              (tuple)
Γ ⊢ (e1, e2, ...) : (T1, T2, ...)

Γ ⊢ e1 : T    Γ ⊢ e2 : T    ...
─────────────────────────────                  (list)
Γ ⊢ [e1, e2, ...] : List[T]

Γ ⊢ f : (T1, T2, ...) -> R    Γ ⊢ args : (T1, T2, ...)
────────────────────────────────────────────────────────  (call)
Γ ⊢ f(args) : R
```

#### Statements
```
Γ ⊢ e : T    Γ' = Γ, x : T
─────────────────────────    (let without annotation)
Γ ⊢ let x = e → Γ'

Γ ⊢ e : T'    T' <: T    Γ' = Γ, x : T
──────────────────────────────────────  (let with annotation)
Γ ⊢ let x: T = e → Γ'

Γ ⊢ e : T    T = current_function.return_type
─────────────────────────────────────────────  (return)
Γ ⊢ return e
```

## Error Types to Detect

1. **Type Mismatch**:
   - Function argument type != parameter type
   - Return type != declared return type
   - If branch types incompatible
   - Binary operator operand types invalid

2. **Undeclared Names**:
   - Variable used before declaration
   - Function not defined

3. **Redeclaration**:
   - Variable declared twice in same scope
   - Function defined multiple times

4. **Arity Mismatch**:
   - Wrong number of function arguments
   - Tuple destructuring size mismatch

5. **Invalid Operations**:
   - Non-boolean condition in if
   - Non-integer list index
   - Arithmetic on non-numeric types

6. **Return Path Analysis**:
   - Missing return in non-Unit function
   - Unreachable code after return

## Testing Strategy

### Unit Tests
- Type system operations (equality, compatibility)
- Symbol table operations (declare, lookup, scope)
- Individual expression type checking
- Individual statement type checking

### Integration Tests
- Full program type checking
- Error recovery and multiple errors
- Type inference across functions
- Complex nested expressions

### Example Programs to Type Check
```lucid
# examples/typed_fibonacci.lucid
function fibonacci(n: Int) returns Int {
    if n <= 1 {
        return n
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2)
    }
}

function main() returns Int {
    let result = fibonacci(10)  # Type inferred as Int
    return result
}
```

## Success Criteria

Phase 3 is complete when:
- [ ] All primitive types (Int, Float, String, Bool) are supported
- [ ] List[T] and Tuple types work correctly
- [ ] All binary and unary operators type check
- [ ] Function calls validate argument types
- [ ] Let statements perform type inference
- [ ] If expressions check branch compatibility
- [ ] Lambda expressions infer types
- [ ] Clear error messages with source locations
- [ ] All example programs type check correctly
- [ ] Comprehensive test suite passes
- [ ] Integration with parser pipeline works

## Performance Targets

- Type check 10,000 lines per second
- Single-pass over AST (after symbol collection)
- Efficient symbol table lookups (O(1) hash map)
- Minimal memory allocation

## Next Phase Preview: Phase 4 - Code Generation

After completing Phase 3, Phase 4 will implement:
1. IR (Intermediate Representation) generation
2. Bytecode compiler
3. Constant folding optimizations
4. Basic optimizations (dead code elimination)

## Notes

- Use visitor pattern consistently with parser
- Leverage existing AST infrastructure
- Follow C++20 modern practices
- Error messages should be helpful, not just correct
- Type inference should be predictable and local
