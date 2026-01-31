# LUCID

A statically-typed, functional-first programming language with Python-inspired syntax, implemented in modern C++20.

## Features

- **Static typing** with type inference
- **Functional-first** design with immutable data structures
- **Python-inspired syntax** that's easy to read and write
- **Stack-based VM** for bytecode execution
- **Built-in collections**: Lists and Tuples
- **File I/O** support
- **1000+ tests** ensuring correctness

## Quick Start

### Building

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake libfmt-dev

# Build
mkdir build && cd build
cmake ..
make -j4

# Run tests
./lucid-tests
```

### Hello World

```python
function main() returns Int {
    println("Hello, World!")
    return 0
}
```

```bash
./lucidc hello.lucid
```

## Language Overview

### Functions

```python
function add(x: Int, y: Int) returns Int {
    return x + y
}

function greet(name: String) returns String {
    return "Hello, " + name + "!"
}
```

### Variables

```python
function main() returns Int {
    let x = 42
    let name = "LUCID"
    let pi = 3.14159
    let flag = true
    return x
}
```

### Control Flow

```python
function max(a: Int, b: Int) returns Int {
    return if a > b { a } else { b }
}

function classify(n: Int) returns String {
    return if n < 0 {
        "negative"
    } else {
        if n == 0 { "zero" } else { "positive" }
    }
}
```

### Collections

```python
function main() returns Int {
    # Lists
    let nums = [1, 2, 3, 4, 5]
    let first = nums[0]
    let len = nums.length()

    # Tuples
    let point = (10, 20)
    let x = point[0]

    return first + x
}
```

### Recursion

```python
function factorial(n: Int) returns Int {
    return if n <= 1 { 1 } else { n * factorial(n - 1) }
}

function fibonacci(n: Int) returns Int {
    return if n <= 1 { n } else { fibonacci(n - 1) + fibonacci(n - 2) }
}
```

### File I/O

```python
function main() returns Int {
    # Write to file
    let ok = write_file("output.txt", "Hello, File!")

    # Read from file
    let content = read_file("output.txt")
    println(content)

    # Check if file exists
    if file_exists("config.txt") {
        println("Config found")
    }

    return 0
}
```

## Built-in Functions

### I/O
| Function | Description |
|----------|-------------|
| `print(value)` | Print without newline |
| `println(value)` | Print with newline |
| `read_file(path)` | Read file contents |
| `write_file(path, content)` | Write to file |
| `append_file(path, content)` | Append to file |
| `file_exists(path)` | Check if file exists |

### Type Conversion
| Function | Description |
|----------|-------------|
| `to_string(value)` | Convert any value to string |

## Methods

### String Methods
| Method | Description |
|--------|-------------|
| `.length()` | Get string length |
| `.is_empty()` | Check if empty |
| `.contains(substr)` | Check for substring |
| `.starts_with(prefix)` | Check prefix |
| `.ends_with(suffix)` | Check suffix |
| `.to_upper()` | Convert to uppercase |
| `.to_lower()` | Convert to lowercase |
| `.trim()` | Remove whitespace |

### List Methods
| Method | Description |
|--------|-------------|
| `.length()` | Get list length |
| `.is_empty()` | Check if empty |
| `.head()` | Get first element |
| `.tail()` | Get all except first |
| `.append(x)` | Add element (returns new list) |
| `.reverse()` | Reverse list |
| `.concat(other)` | Concatenate lists |

### Numeric Methods
| Method | Description |
|--------|-------------|
| `.to_string()` | Convert to string |
| `.abs()` | Absolute value |
| `.floor()` | Floor (Float only) |
| `.ceil()` | Ceiling (Float only) |
| `.round()` | Round (Float only) |

## Types

| Type | Description | Example |
|------|-------------|---------|
| `Int` | 64-bit integer | `42`, `-17` |
| `Float` | 64-bit float | `3.14`, `-0.5` |
| `Bool` | Boolean | `true`, `false` |
| `String` | UTF-8 string | `"hello"` |
| `List[T]` | Homogeneous list | `[1, 2, 3]` |
| `Tuple[T...]` | Fixed-size tuple | `(1, "a", true)` |

## Operators

### Arithmetic
`+`, `-`, `*`, `/`, `%`, `**` (power)

### Comparison
`==`, `!=`, `<`, `>`, `<=`, `>=`

### Logical
`and`, `or`, `not`

## Project Structure

```
lucid/
├── include/lucid/
│   ├── frontend/      # Lexer, Parser, AST
│   ├── semantic/      # Type checker
│   └── backend/       # Compiler, VM, Bytecode
├── src/
│   ├── frontend/
│   ├── semantic/
│   ├── backend/
│   └── main.cpp
├── tests/
└── examples/
```

## Requirements

- C++20 compiler (GCC 10+, Clang 12+)
- CMake 3.20+
- fmt library
- Catch2 3.x (for tests)

## License

MIT License
