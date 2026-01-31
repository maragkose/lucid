# LUCID Language Specification v0.3

**Design Goal**: A statically-typed, functional-first language optimized for AI code generation with Python-inspired natural syntax and strong effect safety.

**Key Design Principles:**
- Natural, readable syntax inspired by Python
- Explicit effects in type signatures
- Strong static typing with inference
- Immutable by default
- Result types instead of exceptions

**Authors**: Initial draft  
**Date**: December 2024  
**Status**: Draft specification

---

## Table of Contents

1. [Core Principles](#1-core-principles)
2. [Lexical Structure](#2-lexical-structure)
3. [Type System](#3-type-system)
4. [Syntax](#4-syntax)
5. [Effect System](#5-effect-system)
6. [Modules](#6-modules)
7. [Standard Library](#7-standard-library)
8. [Complete Examples](#8-complete-examples)
9. [Design Decisions for AI](#9-design-decisions-for-ai)
10. [Comparison with Other Languages](#10-comparison-with-other-languages)

---

## 1. Core Principles

### 1.1 Natural Syntax
- Python-inspired readability with explicit braces
- Clear keywords (`function`, `returns`, `match`, `case`)
- Minimal punctuation noise
- Familiar to both humans and AI models

### 1.2 Type-Driven Development
- Strong static types guide AI code generation
- Required type signatures for top-level functions
- Type inference for local bindings
- Effects visible in function signatures

### 1.3 Safety by Default
- Immutable bindings by default
- No null pointers (use `Option[T]`)
- No exceptions (use `Result[T, E]`)
- Effect tracking prevents hidden side effects

### 1.4 Composability
- Pure functions compose naturally
- Effects chain explicitly with `await`
- Builder pattern for updates
- Pattern matching for control flow

### 1.5 AI-Optimized
- Unambiguous syntax (one way to do things)
- Explicit over implicit
- Local reasoning (no global state)
- Exhaustive pattern matching

---

## 2. Lexical Structure

### 2.1 Identifiers

```
identifier ::= (letter | '_') (letter | digit | '_')*
type_name  ::= UpperCaseLetter (letter | digit | '_')*
```

**Examples:**
- Valid identifiers: `user`, `calculate_tax`, `user_id`, `_temp`
- Valid type names: `User`, `UserId`, `HttpResponse`, `List`

### 2.2 Keywords

```
function    # Function definition
returns     # Return type annotation
type        # Type definition
match       # Pattern matching
case        # Match case
if, else    # Conditionals
let         # Immutable binding
import      # Module import
export      # Module export
from        # Import source
where       # Type constraints
await       # Effect sequencing
return      # Return value
lambda      # Anonymous function
```

### 2.3 Operators

```
# Assignment and binding
=           # Binding
:           # Type annotation

# Comparison
==, !=      # Equality
<, >, <=, >= # Comparison

# Arithmetic
+, -, *, /  # Basic arithmetic
%           # Modulo
**          # Power

# Logical
and, or, not  # Boolean logic

# Special
?           # Error propagation (Result unwrap)
.           # Method call / field access
..          # Range (exclusive)
..=         # Range (inclusive)
```

**Key constraint:** No operator overloading. Each operator has one fixed meaning.

### 2.4 Comments

```python
# Single line comment

#[
  Multi-line
  comment block
]#
```

### 2.5 Literals

```python
# Integers (arbitrary precision)
42
-17
1_000_000      # Underscores for readability

# Floats (64-bit)
3.14
-0.001
1.5e10
2.5e-3

# Strings (UTF-8)
"Hello, World!"
"Line 1\nLine 2"    # Escape sequences: \n \t \r \\ \"

# String interpolation
"User {name} has {age} years"
"Total: {calculate_total(items)}"  # Expressions in braces

# Booleans
true
false

# Unit (void type)
Unit
```

---

## 3. Type System

### 3.1 Primitive Types

```python
Int         # Arbitrary precision integer
Float       # 64-bit floating point
String      # UTF-8 encoded string
Bool        # true or false
Unit        # Empty type (like void)
```

### 3.2 Composite Types

```python
List[T]              # Immutable list
Map[K, V]            # Immutable hash map
Set[T]               # Immutable set
Tuple[T1, T2, ...]   # Fixed-size heterogeneous tuple
```

### 3.3 Effect Types

```python
IO[T]                # Input/output effect
Result[T, E]         # Failable computation
Option[T]            # Optional value
```

### 3.4 Function Types

```python
# Pure function
Int -> Int

# Multiple parameters
(Int, Int) -> Int

# With effects
String -> IO[Result[User, Error]]

# Generic function
[T] (List[T], (T -> Bool)) -> List[T]
```

### 3.5 Type Definitions

#### 3.5.1 Type Aliases

```python
# Simple alias
type UserId = String

# Generic alias
type Pair[A, B] = Tuple[A, B]

# Function type alias
type Handler = Request -> IO[Response]
```

#### 3.5.2 Record Types (Structs)

```python
# Simple record
type User {
    id: UserId,
    name: String,
    email: String,
    age: Int
}

# Generic record
type Response[T] {
    status: Int,
    headers: Map[String, String],
    body: T
}

# Nested types
type Customer {
    user: User,
    address: Address,
    orders: List[Order]
}
```

#### 3.5.3 Anonymous Records

```python
# Anonymous records are allowed
point = {x: 10, y: 20}  # Type inferred as {x: Int, y: Int}

# Can be used in function signatures
function distance(p1: {x: Float, y: Float}, p2: {x: Float, y: Float}) returns Float {
    dx = p1.x - p2.x
    dy = p1.y - p2.y
    return (dx ** 2 + dy ** 2).sqrt()
}

# Pattern matching on anonymous records
match point {
    case {x: 0, y: 0}: "origin"
    case {x: x, y: 0}: "on x-axis"
    case {x: 0, y: y}: "on y-axis"
    case _: "somewhere else"
}
```

#### 3.5.4 Sum Types (Tagged Unions)

```python
# Simple enum
type Status {
    | Pending
    | Active
    | Completed
    | Failed
}

# Sum type with data
type Result[T, E] {
    | Ok(T)
    | Err(E)
}

type Option[T] {
    | Some(T)
    | None
}

# Complex sum type
type PaymentMethod {
    | CreditCard(number: String, exp_month: Int, exp_year: Int)
    | PayPal(email: String)
    | BankTransfer(account: String, routing: String)
    | Cash
}

# Recursive type
type Tree[T] {
    | Leaf(value: T)
    | Node(left: Tree[T], right: Tree[T])
}
```

#### 3.5.5 Constrained Types (Refinement Types)

```python
# Type with constraint using 'where' keyword
type PositiveInt = Int where value > 0

type NonEmptyString = String where value.length() > 0

type Email = String where 
    value.contains("@") and 
    value.contains(".") and
    value.length() > 5

# Range constraint
type Age = Int where value >= 0 and value <= 150

type Percentage = Float where value >= 0.0 and value <= 100.0

# Complex constraint
type Username = String where
    value.length() >= 3 and
    value.length() <= 20 and
    value.matches("[a-zA-Z0-9_]+")
```

### 3.6 Generic Types with Constraints

```python
# Generic function with constraint using 'where' keyword
function max[T](a: T, b: T) returns T where T: Comparable {
    if a > b {
        return a
    }
    return b
}

# Multiple constraints
function sort_and_display[T](items: List[T]) returns IO[Unit] 
where T: Comparable, T: Displayable {
    sorted = items.sort()
    sorted.each(lambda item: print(item))
}

# Constraint on return type
function parse_number[T](s: String) returns Result[T, ParseError]
where T: FromString {
    return T.from_string(s)
}
```

**Note:** Higher-kinded types and dependent types are NOT supported to keep the language simple for AI generation.

---

## 4. Syntax

### 4.1 Function Definition

#### 4.1.1 Basic Function

```python
# Pure function with type signature
function add(x: Int, y: Int) returns Int {
    return x + y
}

# Single expression
function double(x: Int) returns Int {
    return x * 2
}

# Multiple statements
function greet(name: String) returns String {
    message = "Hello, {name}!"
    return message
}
```

#### 4.1.2 Functions with Effects

```python
# IO effect
function print_greeting(name: String) returns IO[Unit] {
    print("Hello, {name}!")
}

# Result type
function divide(x: Int, y: Int) returns Result[Int, String] {
    if y == 0 {
        return Err("Division by zero")
    }
    return Ok(x / y)
}

# Combined effects
function fetch_user(id: UserId) returns IO[Result[User, Error]] {
    response = await http_get("/users/{id}")
    
    match parse_user(response) {
        case Ok(user): return Ok(user)
        case Err(e): return Err(e)
    }
}
```

#### 4.1.3 Generic Functions

```python
# Single type parameter
function identity[T](x: T) returns T {
    return x
}

# Multiple type parameters
function pair[A, B](first: A, second: B) returns Tuple[A, B] {
    return (first, second)
}

# With constraints using 'where'
function min[T](a: T, b: T) returns T where T: Comparable {
    if a < b {
        return a
    }
    return b
}

# Multiple constraints
function sort_and_show[T](items: List[T]) returns String 
where T: Comparable, T: Display {
    sorted = items.sort()
    return sorted.map(lambda x: x.to_string()).join(", ")
}
```

### 4.2 Multiple Return Values with Destructuring

```python
# Function returning tuple
function divide_with_remainder(x: Int, y: Int) returns Tuple[Int, Int] {
    quotient = x / y
    remainder = x % y
    return (quotient, remainder)
}

# Destructuring on call
(quotient, remainder) = divide_with_remainder(10, 3)
println("Quotient: {quotient}, Remainder: {remainder}")

# Destructuring in let binding
let (width, height) = get_dimensions()

# Pattern matching destructuring
match calculate_stats(data) {
    case (mean, stddev, count): {
        println("Mean: {mean}, StdDev: {stddev}, Count: {count}")
    }
}

# Partial destructuring (ignore some values)
(first, _, third) = get_three_values()
```

### 4.3 Record Updates with Builder Pattern

```python
# Original record
user = User {
    id: "user-123",
    name: "Alice",
    email: "alice@example.com",
    age: 30
}

# Builder pattern - creates new record with updated field
updated_user = user.with_age(31)

# Multiple updates
updated_user = user
    .with_name("Alice Smith")
    .with_age(31)
    .with_email("alice.smith@example.com")

# Conditional updates
final_user = if needs_update {
    user.with_age(user.age + 1)
} else {
    user
}

# Builder methods are auto-generated for all record types
type Point {
    x: Float,
    y: Float
}

# Automatically available:
# - point.with_x(new_value) -> Point
# - point.with_y(new_value) -> Point

point = Point {x: 1.0, y: 2.0}
moved = point.with_x(5.0)  # Point {x: 5.0, y: 2.0}
```

### 4.4 Lambda Functions

```python
# Simple lambda
add_one = lambda x: x + 1

# Multiple parameters
add = lambda x, y: x + y

# Multi-line lambda
complex_transform = lambda item: {
    validated = validate(item)
    transformed = transform(validated)
    return formatted(transformed)
}

# Used in higher-order functions
numbers = [1, 2, 3, 4, 5]
doubled = numbers.map(lambda x: x * 2)
evens = numbers.filter(lambda x: x % 2 == 0)
```

### 4.5 Let Bindings

```python
# Immutable binding (default)
let x = 42
let message = "Hello"

# Type annotation (optional)
let count: Int = 10
let name: String = "Alice"

# Destructuring tuples
let (first, second) = (1, 2)

# Destructuring records
let {name, age} = user
let {id, name, email} = user

# Destructuring with renaming
let {name as user_name, age as user_age} = user

# Partial destructuring
let {name} = user  # Only extract name
```

### 4.6 Control Flow

#### 4.6.1 If Expression

```python
# Both branches must return same type
result = if x > 0 {
    "positive"
} else {
    "non-positive"
}

# Nested conditions
category = if age < 13 {
    "child"
} else if age < 20 {
    "teenager"
} else {
    "adult"
}

# If without else returns Option[T]
value = if condition {
    42
}  # Type: Option[Int]
```

#### 4.6.2 Match Expression

```python
# Exhaustive pattern matching
category = match age {
    case 0..13: "child"
    case 13..20: "teenager"
    case 20..65: "adult"
    case 65..: "senior"
}

# Match on sum types
message = match result {
    case Ok(value): "Got: {value}"
    case Err(error): "Failed: {error}"
}

# Match with guards
description = match value {
    case x if x < 0: "negative"
    case 0: "zero"
    case x if x > 0: "positive"
}

# Destructuring in patterns
match user {
    case {name: "Admin", age: age}: "Admin user, age {age}"
    case {name: name, age: age} if age >= 18: "Adult: {name}"
    case {name: name}: "Minor: {name}"
}

# Tuple patterns
result = match (status, user_type) {
    case (Active, Premium): "active_premium"
    case (Active, Free): "active_free"
    case (Inactive, _): "inactive"
}
```

### 4.7 Method Chaining and Pipelines

```python
# Method chaining (preferred style)
result = items
    .filter(lambda x: x.is_valid())
    .map(lambda x: x.transform())
    .sort()
    .take(10)

# Builder pattern chaining
user = User {id: "1", name: "Alice", email: "a@b.com", age: 30}
    .with_name("Alice Smith")
    .with_age(31)
```

### 4.8 Error Handling with ? Operator

```python
# The ? operator propagates errors
function create_user(name: String, email: String, age: Int) returns Result[User, Error] {
    # Each ? unwraps Ok or returns early with Err
    validated_email = validate_email(email)?
    validated_age = validate_age(age)?
    validated_name = validate_name(name)?
    
    user = User {
        id: generate_id(),
        name: validated_name,
        email: validated_email,
        age: validated_age
    }
    
    return Ok(user)
}
```

### 4.9 Effect Sequencing with await

```python
# Sequential IO operations
function process_order(order_id: String) returns IO[Result[Receipt, Error]] {
    # Each await sequences the IO effects
    user = await fetch_user(order_id)
    
    match validate_user(user) {
        case Ok(validated): {
            payment = await charge_payment(validated)
            receipt = await generate_receipt(payment)
            await send_confirmation(receipt)
            return Ok(receipt)
        }
        case Err(e): {
            return Err(e)
        }
    }
}

# Combining await and ?
function fetch_and_process(id: String) returns IO[Result[ProcessedData, Error]] {
    # await for IO, ? for Result
    raw = (await fetch_raw_data(id))?
    parsed = parse_data(raw)?
    processed = await process_in_db(parsed)
    return processed
}
```

---

## 5. Effect System

### 5.1 Core Effect Types

LUCID has three built-in effect types. No custom effects are supported (for simplicity).

```python
# IO effect - external world interaction
IO[T]

# Result effect - failable computation
Result[T, E]

# Option effect - optional value
Option[T]
```

### 5.2 IO Effect

The `IO[T]` type marks functions that interact with the external world.

```python
# Console I/O
function print(message: String) returns IO[Unit]
function println(message: String) returns IO[Unit]
function read_line() returns IO[String]

# File I/O
function read_file(path: String) returns IO[Result[String, Error]]
function write_file(path: String, content: String) returns IO[Result[Unit, Error]]

# Network
function http_get(url: String) returns IO[Result[String, HttpError]]
function http_post(url: String, body: String) returns IO[Result[String, HttpError]]

# Lift pure value into IO
function pure[T](value: T) returns IO[T]
```

#### Example: File Processing

```python
function process_file(input_path: String, output_path: String) returns IO[Result[Unit, Error]] {
    content = await read_file(input_path)
    
    match content {
        case Ok(text): {
            # Pure transformation
            processed = text
                .split("\n")
                .filter(lambda line: line.length() > 0)
                .map(lambda line: line.trim())
                .join("\n")
            
            # IO operation
            result = await write_file(output_path, processed)
            return result
        }
        case Err(e): {
            return Err(e)
        }
    }
}
```

### 5.3 Result Effect

```python
# Success or failure (standard library type)
type Result[T, E] {
    | Ok(T)
    | Err(E)
}

# Result operations
function map[A, B, E](f: A -> B, result: Result[A, E]) returns Result[B, E]
function and_then[A, B, E](f: A -> Result[B, E], result: Result[A, E]) returns Result[B, E]
function unwrap_or[T, E](default: T, result: Result[T, E]) returns T
```

#### Example: Validation Chain

```python
function validate_user(user: User) returns Result[User, ValidationError] {
    # Using ? operator
    validate_email(user.email)?
    validate_age(user.age)?
    validate_name(user.name)?
    return Ok(user)
}
```

### 5.4 Option Effect

```python
# Value or nothing (standard library type)
type Option[T] {
    | Some(T)
    | None
}

# Option operations
function map[A, B](f: A -> B, opt: Option[A]) returns Option[B]
function and_then[A, B](f: A -> Option[B], opt: Option[A]) returns Option[B]
function unwrap_or[T](default: T, opt: Option[T]) returns T
```

#### Example: Safe Dictionary Lookup

```python
function get_user_email(users: Map[UserId, User], id: UserId) returns Option[String] {
    return users.get(id).map(lambda u: u.email)
}
```

### 5.5 Combining Effects

```python
# IO + Result
function fetch_and_parse(url: String) returns IO[Result[Data, Error]] {
    response = await http_get(url)
    
    match response {
        case Ok(text): {
            return parse_data(text)
        }
        case Err(e): {
            return Err(e)
        }
    }
}

# All three effects
function get_user_data(id: UserId) returns IO[Result[Option[UserData], Error]] {
    user_result = await fetch_user(id)
    
    match user_result {
        case Ok(user): {
            # user might not have data
            data = user.get_optional_data()
            return Ok(Some(data))
        }
        case Err(NotFound): {
            return Ok(None)
        }
        case Err(e): {
            return Err(e)
        }
    }
}
```

---

## 6. Modules

### 6.1 Module Definition

```python
# file: user.lucid

# Export types
export type UserId = String

export type User {
    id: UserId,
    name: String,
    email: String,
    age: Int
}

export type UserError {
    | InvalidEmail(String)
    | InvalidAge(String)
    | NotFound(UserId)
}

# Export functions
export function create_user(name: String, email: String, age: Int) returns Result[User, UserError] {
    validated_email = validate_email(email)?
    validated_age = validate_age(age)?
    
    user = User {
        id: generate_id(),
        name: name,
        email: validated_email,
        age: validated_age
    }
    
    return Ok(user)
}

# Private function (not exported)
function validate_email(email: String) returns Result[String, UserError] {
    if email.contains("@") and email.contains(".") {
        return Ok(email)
    }
    return Err(InvalidEmail("Email must contain @ and ."))
}
```

### 6.2 Module Import

```python
# Import entire module
import User from "user"
# Usage: User.create_user(...)

# Import specific items
import {create_user, User, UserId} from "user"
# Usage: create_user(...)

# Import with alias
import Database as DB from "database"
# Usage: DB.query(...)
```

---

## 7. Standard Library

### 7.1 Core Types

```python
# Primitives
Int, Float, String, Bool, Unit

# Collections (immutable)
List[T], Map[K, V], Set[T], Tuple[T1, T2, ...]

# Effects
IO[T], Result[T, E], Option[T]
```

### 7.2 List Operations

```python
# Core operations
function length[T](list: List[T]) returns Int
function head[T](list: List[T]) returns Option[T]
function tail[T](list: List[T]) returns Option[List[T]]
function is_empty[T](list: List[T]) returns Bool

# Transformations
function map[A, B](f: A -> B, list: List[A]) returns List[B]
function filter[T](predicate: T -> Bool, list: List[T]) returns List[T]
function fold[A, B](f: (B, A) -> B, initial: B, list: List[A]) returns B

# Access
function get[T](index: Int, list: List[T]) returns Option[T]
function take[T](n: Int, list: List[T]) returns List[T]
function drop[T](n: Int, list: List[T]) returns List[T]

# Combinations
function concat[T](list1: List[T], list2: List[T]) returns List[T]
function zip[A, B](list1: List[A], list2: List[B]) returns List[Tuple[A, B]]

# Sorting (requires Comparable constraint)
function sort[T](list: List[T]) returns List[T] where T: Comparable
function reverse[T](list: List[T]) returns List[T]

# Aggregations
function sum(list: List[Int]) returns Int
function sum_floats(list: List[Float]) returns Float
function min[T](list: List[T]) returns Option[T] where T: Comparable
function max[T](list: List[T]) returns Option[T] where T: Comparable

# Predicates
function all[T](predicate: T -> Bool, list: List[T]) returns Bool
function any[T](predicate: T -> Bool, list: List[T]) returns Bool
function find[T](predicate: T -> Bool, list: List[T]) returns Option[T]
```

### 7.3 Map Operations

```python
# Creation
function empty[K, V]() returns Map[K, V]
function from_list[K, V](pairs: List[Tuple[K, V]]) returns Map[K, V]

# Access
function get[K, V](key: K, map: Map[K, V]) returns Option[V]
function contains_key[K, V](key: K, map: Map[K, V]) returns Bool

# Modification (returns new map - immutable)
function insert[K, V](key: K, value: V, map: Map[K, V]) returns Map[K, V]
function remove[K, V](key: K, map: Map[K, V]) returns Map[K, V]
function update[K, V](key: K, f: V -> V, map: Map[K, V]) returns Map[K, V]

# Queries
function size[K, V](map: Map[K, V]) returns Int
function is_empty[K, V](map: Map[K, V]) returns Bool
function keys[K, V](map: Map[K, V]) returns List[K]
function values[K, V](map: Map[K, V]) returns List[V]
function entries[K, V](map: Map[K, V]) returns List[Tuple[K, V]]

# Transformations
function map_values[K, V1, V2](f: V1 -> V2, map: Map[K, V1]) returns Map[K, V2]
```

### 7.4 Set Operations

```python
# Creation
function empty[T]() returns Set[T]
function from_list[T](items: List[T]) returns Set[T]

# Membership
function contains[T](value: T, set: Set[T]) returns Bool

# Modification (returns new set - immutable)
function add[T](value: T, set: Set[T]) returns Set[T]
function remove[T](value: T, set: Set[T]) returns Set[T]

# Set operations
function union[T](set1: Set[T], set2: Set[T]) returns Set[T]
function intersection[T](set1: Set[T], set2: Set[T]) returns Set[T]
function difference[T](set1: Set[T], set2: Set[T]) returns Set[T]

# Queries
function size[T](set: Set[T]) returns Int
function is_empty[T](set: Set[T]) returns Bool
function is_subset[T](set1: Set[T], set2: Set[T]) returns Bool
```

### 7.5 String Operations

```python
# Queries
function length(s: String) returns Int
function is_empty(s: String) returns Bool
function contains(substring: String, s: String) returns Bool
function starts_with(prefix: String, s: String) returns Bool
function ends_with(suffix: String, s: String) returns Bool

# Transformations
function to_upper(s: String) returns String
function to_lower(s: String) returns String
function trim(s: String) returns String

# Splitting and joining
function split(delimiter: String, s: String) returns List[String]
function join(delimiter: String, strings: List[String]) returns String

# Replacement
function replace(from: String, to: String, s: String) returns String

# Parsing
function parse_int(s: String) returns Result[Int, ParseError]
function parse_float(s: String) returns Result[Float, ParseError]

# Pattern matching
function matches(pattern: String, s: String) returns Bool
```

### 7.6 IO Operations

```python
# Console I/O
function print(message: String) returns IO[Unit]
function println(message: String) returns IO[Unit]
function eprint(message: String) returns IO[Unit]  # stderr
function eprintln(message: String) returns IO[Unit]
function read_line() returns IO[String]

# File I/O
function read_file(path: String) returns IO[Result[String, IOError]]
function write_file(path: String, content: String) returns IO[Result[Unit, IOError]]
function append_file(path: String, content: String) returns IO[Result[Unit, IOError]]

# File system
function exists(path: String) returns IO[Bool]
function is_file(path: String) returns IO[Bool]
function is_directory(path: String) returns IO[Bool]
function list_directory(path: String) returns IO[Result[List[String], IOError]]

# HTTP (standard library)
function http_get(url: String) returns IO[Result[String, HttpError]]
function http_post(url: String, body: String) returns IO[Result[String, HttpError]]
```

### 7.7 Result Operations

```python
# Transformations
function map[A, B, E](f: A -> B, result: Result[A, E]) returns Result[B, E]
function map_err[T, E1, E2](f: E1 -> E2, result: Result[T, E1]) returns Result[T, E2]
function and_then[A, B, E](f: A -> Result[B, E], result: Result[A, E]) returns Result[B, E]

# Queries
function is_ok[T, E](result: Result[T, E]) returns Bool
function is_err[T, E](result: Result[T, E]) returns Bool

# Extraction
function unwrap[T, E](result: Result[T, E]) returns T  # Panics on Err
function unwrap_or[T, E](default: T, result: Result[T, E]) returns T
function unwrap_or_else[T, E](f: E -> T, result: Result[T, E]) returns T

# Conversion
function ok[T, E](result: Result[T, E]) returns Option[T]
function err[T, E](result: Result[T, E]) returns Option[E]
```

### 7.8 Option Operations

```python
# Transformations
function map[A, B](f: A -> B, opt: Option[A]) returns Option[B]
function and_then[A, B](f: A -> Option[B], opt: Option[A]) returns Option[B]
function filter[T](predicate: T -> Bool, opt: Option[T]) returns Option[T]

# Queries
function is_some[T](opt: Option[T]) returns Bool
function is_none[T](opt: Option[T]) returns Bool

# Extraction
function unwrap[T](opt: Option[T]) returns T  # Panics on None
function unwrap_or[T](default: T, opt: Option[T]) returns T
function unwrap_or_else[T](f: Unit -> T, opt: Option[T]) returns T

# Conversion
function ok_or[T, E](error: E, opt: Option[T]) returns Result[T, E]
```

---

## 8. Complete Examples

### 8.1 Hello World

```python
function main() returns IO[Unit] {
    println("Hello, World!")
}
```

### 8.2 Using Builder Pattern

```python
type User {
    id: String,
    name: String,
    email: String,
    age: Int,
    is_active: Bool
}

function create_and_update_user() returns User {
    # Create initial user
    user = User {
        id: "user-123",
        name: "Alice",
        email: "alice@example.com",
        age: 30,
        is_active: true
    }
    
    # Update using builder pattern
    updated = user
        .with_name("Alice Smith")
        .with_age(31)
        .with_email("alice.smith@example.com")
    
    return updated
}

function main() returns IO[Unit] {
    user = create_and_update_user()
    println("User: {user.name}, Age: {user.age}")
}
```

### 8.3 Using Destructuring

```python
function calculate_stats(numbers: List[Int]) returns Tuple[Float, Float, Int] {
    count = numbers.length()
    sum = numbers.sum()
    mean = sum.to_float() / count.to_float()
    
    variance = numbers
        .map(lambda x: (x.to_float() - mean) ** 2)
        .sum_floats() / count.to_float()
    
    stddev = variance.sqrt()
    
    return (mean, stddev, count)
}

function main() returns IO[Unit] {
    numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    
    # Destructure tuple result
    (mean, stddev, count) = calculate_stats(numbers)
    
    println("Count: {count}")
    println("Mean: {mean}")
    println("StdDev: {stddev}")
}
```

### 8.4 Using Anonymous Records

```python
# Anonymous records for quick data structures
function create_point(x: Float, y: Float) returns {x: Float, y: Float} {
    return {x: x, y: y}
}

function distance(p1: {x: Float, y: Float}, p2: {x: Float, y: Float}) returns Float {
    dx = p1.x - p2.x
    dy = p1.y - p2.y
    return (dx ** 2 + dy ** 2).sqrt()
}

function main() returns IO[Unit] {
    origin = {x: 0.0, y: 0.0}
    point = {x: 3.0, y: 4.0}
    
    dist = distance(origin, point)
    println("Distance: {dist}")  # Should be 5.0
}
```

### 8.5 Using Generic Constraints

```python
# Find minimum with Comparable constraint
function find_min[T](items: List[T]) returns Option[T] where T: Comparable {
    if items.is_empty() {
        return None
    }
    
    return items.fold(
        lambda acc, item: if item < acc { item } else { acc },
        items.head().unwrap()
    ).into_some()
}

# Sort and display with multiple constraints
function sort_and_display[T](items: List[T]) returns IO[Unit] 
where T: Comparable, T: Display {
    sorted = items.sort()
    
    sorted.each(lambda item: {
        println(item.to_string())
    })
}

function main() returns IO[Unit] {
    numbers = [5, 2, 8, 1, 9, 3]
    
    match find_min(numbers) {
        case Some(min): println("Minimum: {min}")
        case None: println("Empty list")
    }
    
    await sort_and_display(numbers)
}
```

### 8.6 Complete User Management System

```python
# Types
type UserId = String

type Email = String where 
    value.contains("@") and 
    value.contains(".")

type User {
    id: UserId,
    name: String,
    email: Email,
    age: Int,
    created_at: Int
}

type UserError {
    | ValidationError(String)
    | DatabaseError(String)
    | NotFound(UserId)
}

# Validation functions
function validate_email(email: String) returns Result[Email, UserError] {
    if email.contains("@") and email.contains(".") and email.length() > 5 {
        return Ok(email)
    }
    return Err(ValidationError("Invalid email format"))
}

function validate_age(age: Int) returns Result[Int, UserError] {
    if age >= 0 and age <= 150 {
        return Ok(age)
    }
    return Err(ValidationError("Age must be between 0 and 150"))
}

function validate_name(name: String) returns Result[String, UserError] {
    trimmed = name.trim()
    if trimmed.length() >= 2 and trimmed.length() <= 100 {
        return Ok(trimmed)
    }
    return Err(ValidationError("Name must be 2-100 characters"))
}

# CRUD operations
function create_user(name: String, email: String, age: Int) returns IO[Result[User, UserError]] {
    # Validate all fields
    validated_name = validate_name(name)?
    validated_email = validate_email(email)?
    validated_age = validate_age(age)?
    
    # Create user
    user = User {
        id: generate_uuid(),
        name: validated_name,
        email: validated_email,
        age: validated_age,
        created_at: current_timestamp()
    }
    
    # Save to database
    result = await save_to_db(user)
    
    match result {
        case Ok(_): {
            println("User created: {user.id}")
            return Ok(user)
        }
        case Err(e): {
            return Err(DatabaseError(e))
        }
    }
}

function find_user(id: UserId) returns IO[Result[User, UserError]] {
    query = "SELECT * FROM users WHERE id = ?"
    result = await db_query(query, [id])
    
    match result {
        case Ok(rows): {
            match rows {
                case []: return Err(NotFound(id))
                case [row]: return Ok(parse_user_row(row))
                case _: return Err(DatabaseError("Multiple users with same ID"))
            }
        }
        case Err(e): {
            return Err(DatabaseError(e))
        }
    }
}

function update_user(id: UserId, name: Option[String], age: Option[Int]) returns IO[Result[User, UserError]] {
    # Fetch existing user
    existing = (await find_user(id))?
    
    # Build updated user using builder pattern
    updated = existing
    
    # Apply updates if provided
    updated = match name {
        case Some(n): {
            validated = validate_name(n)?
            updated.with_name(validated)
        }
        case None: updated
    }
    
    updated = match age {
        case Some(a): {
            validated = validate_age(a)?
            updated.with_age(validated)
        }
        case None: updated
    }
    
    # Save to database
    query = "UPDATE users SET name = ?, age = ? WHERE id = ?"
    (await db_execute(query, [updated.name, updated.age, updated.id]))?
    
    return Ok(updated)
}

function delete_user(id: UserId) returns IO[Result[Unit, UserError]] {
    # Check if user exists
    (await find_user(id))?
    
    # Delete
    query = "DELETE FROM users WHERE id = ?"
    result = await db_execute(query, [id])
    
    match result {
        case Ok(_): return Ok(Unit)
        case Err(e): return Err(DatabaseError(e))
    }
}

# Main function
function main() returns IO[Unit] {
    # Create a user
    result = await create_user("Alice Smith", "alice@example.com", 30)
    
    match result {
        case Ok(user): {
            println("Created user: {user.id}")
            
            # Update the user using builder pattern and Option
            updated = await update_user(user.id, Some("Alice Johnson"), None)
            
            match updated {
                case Ok(u): println("Updated user: {u.name}")
                case Err(e): eprintln("Update failed: {e}")
            }
            
            # Find with destructuring
            match await find_user(user.id) {
                case Ok({name, email, age}): {
                    println("Found: {name}, {email}, age {age}")
                }
                case Err(e): {
                    eprintln("Error: {e}")
                }
            }
        }
        case Err(e): {
            eprintln("Error creating user: {e}")
        }
    }
}

# Helper functions
function generate_uuid() returns String {
    return "uuid-{random()}"
}

function current_timestamp() returns Int {
    return 1234567890
}

function parse_user_row(row: DbRow) returns User {
    # Implementation details omitted
    return User {
        id: row.get("id"),
        name: row.get("name"),
        email: row.get("email"),
        age: row.get_int("age"),
        created_at: row.get_int("created_at")
    }
}
```

### 8.7 Data Processing Pipeline

```python
type RawData {
    id: String,
    value: Float,
    timestamp: Int,
    category: String
}

type ProcessedData {
    id: String,
    normalized_value: Float,
    category: String,
    quality_score: Float
}

type AggregateStats {
    category: String,
    count: Int,
    total: Float,
    average: Float,
    min: Float,
    max: Float
}

function parse_data(raw: String) returns Result[RawData, String] {
    json = parse_json(raw)?
    
    data = RawData {
        id: json.get("id")?,
        value: parse_float(json.get("value")?)?,
        timestamp: parse_int(json.get("timestamp")?)?,
        category: json.get("category")?
    }
    
    return Ok(data)
}

function validate_data(data: RawData) returns Result[RawData, String] {
    if data.value < 0.0 {
        return Err("Value must be non-negative")
    }
    
    if data.timestamp < 0 {
        return Err("Timestamp must be non-negative")
    }
    
    if data.category.length() == 0 {
        return Err("Category cannot be empty")
    }
    
    return Ok(data)
}

function process_data(data: RawData) returns ProcessedData {
    normalized = data.value / 100.0
    age_days = (current_timestamp() - data.timestamp) / 86400
    quality = if age_days < 7 { 1.0 } else { 0.8 }
    
    return ProcessedData {
        id: data.id,
        normalized_value: normalized,
        category: data.category,
        quality_score: quality
    }
}

function aggregate_by_category(data: List[ProcessedData]) returns Map[String, AggregateStats] {
    grouped = data.group_by(lambda d: d.category)
    
    return grouped.map_values(lambda group: {
        values = group.map(lambda d: d.normalized_value)
        
        return AggregateStats {
            category: group.head().unwrap().category,
            count: group.length(),
            total: values.sum_floats(),
            average: values.sum_floats() / group.length().to_float(),
            min: values.min().unwrap_or(0.0),
            max: values.max().unwrap_or(0.0)
        }
    })
}

function run_pipeline(input_file: String, output_file: String) returns IO[Result[Unit, String]] {
    # Read input
    content = (await read_file(input_file))?
    
    # Parse and process
    lines = content.split("\n")
    
    results = lines
        .map(parse_data)
        .filter(Result.is_ok)
        .map(Result.unwrap)
        .map(validate_data)
        .filter(Result.is_ok)
        .map(Result.unwrap)
        .map(process_data)
    
    # Aggregate
    aggregated = aggregate_by_category(results)
    
    # Format output with destructuring
    output = aggregated
        .entries()
        .map(lambda (cat, stats): {
            # Destructure stats in lambda
            AggregateStats {count, average, min, max} = stats
            return "{cat}: count={count}, avg={average:.2f}, min={min:.2f}, max={max:.2f}"
        })
        .join("\n")
    
    # Write output
    (await write_file(output_file, output))?
    
    println("Pipeline completed. Processed {results.length()} records.")
    return Ok(Unit)
}

function main() returns IO[Unit] {
    result = await run_pipeline("data.json", "output.txt")
    
    match result {
        case Ok(_): println("Success!")
        case Err(e): eprintln("Error: {e}")
    }
}
```

---

## 9. Design Decisions for AI

### 9.1 Decided Features Summary

| Feature | Decision | Rationale |
|---------|----------|-----------|
| Function keyword | `function` | More explicit than `def` or `fn` |
| Block delimiters | `{}` braces | Explicit scope, no indentation counting |
| Return type syntax | `returns` | Clear, readable keyword |
| Record updates | Builder pattern | Immutable, chainable, clear |
| Multiple returns | Destructuring | Natural, like Python/JS |
| Anonymous records | Yes | Flexible for quick data structures |
| Generic constraints | `where` keyword | Clear, separate from parameters |
| Higher-kinded types | No | Too complex for AI |
| Dependent types | No | Too complex for AI |
| Effect extensions | Not yet | Keep simple for now |
| Collections | List, Map, Set | Sufficient for most use cases |
| Numeric types | Int, Float | Simple, covers most needs |

### 9.2 Why These Choices Work for AI

#### Builder Pattern for Updates
```python
# AI can generate clear, immutable update chains
user = fetch_user(id)
    .with_name("New Name")
    .with_age(31)
    .with_email("new@email.com")

# Each step is:
# - Explicit (no hidden mutations)
# - Chainable (clear data flow)
# - Type-safe (returns same type)
```

#### Destructuring for Multiple Returns
```python
# AI naturally generates tuple unpacking
(mean, stddev, count) = calculate_stats(data)

# Instead of awkward workarounds like:
stats = calculate_stats(data)
mean = stats[0]    # Error-prone indexing
stddev = stats[1]
count = stats[2]
```

#### Anonymous Records for Flexibility
```python
# AI can quickly create data structures without defining types
point = {x: 10, y: 20}
response = {status: 200, body: "OK"}

# Good for:
# - Quick prototyping
# - Internal data structures
# - Function return values
```

#### Where Constraints for Clarity
```python
# Constraint is separate from parameters
function max[T](a: T, b: T) returns T where T: Comparable {
    if a > b { return a }
    return b
}

# AI knows:
# - T is a type parameter
# - T must support comparison
# - Constraint is explicit, not hidden
```

### 9.3 AI Generation Strategy

1. **Write type signatures first** (guides implementation)
2. **Use builder pattern** for all updates (immutable)
3. **Destructure tuples** when returning multiple values
4. **Apply constraints** with `where` keyword
5. **Chain methods** for data transformations

---

## 10. Comparison with Other Languages

### 10.1 Feature Comparison Table

| Feature | Python | Rust | Haskell | TypeScript | LUCID |
|---------|--------|------|---------|------------|-------|
| Type Safety | Optional | Strong | Strong | Optional | Strong |
| Effect System | No | No | Yes (IO) | No | Yes |
| Result Type | No | Yes | Yes | No | Yes |
| Builder Pattern | Manual | Via traits | Lenses | Manual | Built-in |
| Destructuring | Yes | Yes | Yes | Yes | Yes |
| Anonymous Records | Dict | No | No | Yes | Yes |
| Constraints | Protocols | Traits | Type classes | Interfaces | Where clauses |
| Immutable Default | No | No | Yes | No | Yes |

### 10.2 Syntax Comparison

#### Record Updates

```python
# Python (mutable)
user.name = "Alice"
user.age = 31

# Rust (requires custom impl)
let user = user.with_name("Alice").with_age(31);

# LUCID (built-in)
user = user.with_name("Alice").with_age(31)
```

#### Multiple Returns

```python
# Python
def stats(data):
    return mean, stddev, count
mean, stddev, count = stats(data)

# Rust
fn stats(data: &[i32]) -> (f64, f64, usize) {
    (mean, stddev, count)
}
let (mean, stddev, count) = stats(&data);

# LUCID
function stats(data: List[Int]) returns Tuple[Float, Float, Int] {
    return (mean, stddev, count)
}
(mean, stddev, count) = stats(data)
```

#### Generic Constraints

```python
# Python (duck typing, no enforcement)
def max(a, b):
    return a if a > b else b

# Rust
fn max<T: Ord>(a: T, b: T) -> T {
    if a > b { a } else { b }
}

# Haskell
max :: Ord a => a -> a -> a
max a b = if a > b then a else b

# LUCID
function max[T](a: T, b: T) returns T where T: Comparable {
    if a > b { return a }
    return b
}
```

---

## Appendix A: Quick Reference

### Keywords
```
function, returns, type, match, case, if, else, let,
import, export, from, where, await, return, lambda
```

### Operators
```
=, :, ==, !=, <, >, <=, >=, +, -, *, /, %, **,
and, or, not, ?, ., .., ..=
```

### Built-in Types
```
# Primitives
Int, Float, String, Bool, Unit

# Collections
List[T], Map[K,V], Set[T], Tuple[...]

# Effects
IO[T], Result[T,E], Option[T]
```

### Common Patterns

#### Function Definition
```python
function name[T](param: Type) returns ReturnType where T: Constraint {
    return value
}
```

#### Builder Pattern
```python
updated = record
    .with_field1(value1)
    .with_field2(value2)
```

#### Destructuring
```python
(a, b, c) = function_returning_tuple()
let {name, age} = user
```

#### Pattern Matching
```python
match value {
    case Pattern1: expression1
    case Pattern2: expression2
    case _: default
}
```

#### Error Handling
```python
result = risky_operation()?
# Or
match risky_operation() {
    case Ok(val): use_value(val)
    case Err(e): handle_error(e)
}
```

#### IO Sequencing
```python
data = await fetch_data()
processed = process(data)
await save_data(processed)
```

---

## Appendix B: Grammar Summary

```
program     ::= (import | type_def | function_def)*

import      ::= 'import' (identifier | '{' id_list '}') 'from' string

type_def    ::= 'export'? 'type' TypeName type_params? '=' type_expr
              | 'export'? 'type' TypeName type_params? '{' field_list '}'
              | 'export'? 'type' TypeName type_params? '{' variant_list '}'

function_def ::= 'export'? 'function' identifier type_params? 
                 '(' param_list ')' 'returns' type where_clause? '{' statements '}'

where_clause ::= 'where' constraint (',' constraint)*

constraint  ::= TypeVar ':' trait_name

statement   ::= 'let' pattern '=' expr
              | 'return' expr
              | expr
              | 'if' expr '{' statements '}' ('else' '{' statements '}')?
              | 'match' expr '{' match_arms '}'

expr        ::= literal
              | identifier
              | '{' field_assigns '}'  # Anonymous record
              | 'lambda' param_list ':' expr
              | 'await' expr
              | expr '?'
              | expr '.' identifier  # Method or builder
              | expr '(' arg_list ')'
              | expr binary_op expr
              | unary_op expr
              | '(' expr (',' expr)* ')'  # Tuple

pattern     ::= identifier
              | '_'
              | '(' pattern (',' pattern)* ')'  # Tuple destructuring
              | '{' field_patterns '}'  # Record destructuring
```

---

## Document History

- **v0.1** (December 2024): Initial functional syntax
- **v0.2** (December 2024): Python-inspired syntax with `function`, `returns`, braces
- **v0.3** (December 2024): Decisions finalized:
  - Builder pattern for record updates
  - Destructuring for multiple returns
  - Anonymous records supported
  - Generic constraints with `where` keyword
  - No higher-kinded types or dependent types
  - Collections: List, Map, Set
  - Numeric types: Int, Float

---

**End of Specification v0.3**
