# LUCID Compiler - Phase 7: File I/O

**Status**: ✅ COMPLETE

## Overview
Add file input/output operations to make LUCID programs practically useful for real-world tasks like reading configuration files, processing data files, and generating output.

## Context
- **Input**: Working VM with print/println from Phase 6 (1003 tests passing)
- **Output**: Programs that can read and write files
- **Scope**: Basic file operations (read, write, append, exists)
- **Simplification**: Return Bool for success/failure (no Result type yet)

## Goals

### File Operations
1. `read_file(path: String) -> String` - Read entire file contents
2. `write_file(path: String, content: String) -> Bool` - Write/overwrite file
3. `append_file(path: String, content: String) -> Bool` - Append to file
4. `file_exists(path: String) -> Bool` - Check if file exists

### Design Decisions

**Error Handling**: Since we don't have Result/Option types yet, we'll use:
- `read_file` returns empty string on error (and prints error to stderr)
- `write_file`/`append_file` return `false` on error
- `file_exists` returns `false` if path doesn't exist or on error

**Simplicity**: No directory operations, no binary files, just text file I/O.

---

## Implementation Plan

### Step 1: Add Builtin IDs
Add new builtin function IDs to bytecode.hpp:
```cpp
enum class BuiltinId : uint16_t {
    PRINT = 0,
    PRINTLN = 1,
    TO_STRING = 2,
    READ_FILE = 3,
    WRITE_FILE = 4,
    APPEND_FILE = 5,
    FILE_EXISTS = 6,
};
```

### Step 2: Update Type Checker
Add type signatures for file operations in type_checker.cpp.

### Step 3: Update Compiler
Recognize file builtins and emit CALL_BUILTIN.

### Step 4: Implement in VM
Add file I/O implementations using C++ standard library:
- `std::ifstream` for reading
- `std::ofstream` for writing
- `std::filesystem::exists` for checking existence

### Step 5: Add Tests
Test all file operations with temporary files.

---

## API Specification

```lucid
// Read entire file as string
// Returns empty string if file doesn't exist or on error
let content = read_file("input.txt")

// Write string to file (creates or overwrites)
// Returns true on success, false on error
let success = write_file("output.txt", "Hello, World!")

// Append string to file (creates if doesn't exist)
// Returns true on success, false on error
let success = append_file("log.txt", "New line\n")

// Check if file exists
// Returns true if file exists, false otherwise
let exists = file_exists("config.txt")
```

---

## Example Programs

### Copy File
```lucid
function main() returns Int {
    let content = read_file("source.txt")
    if content.length() > 0 {
        let success = write_file("dest.txt", content)
        if success {
            println("File copied successfully")
            return 0
        }
    }
    println("Failed to copy file")
    return 1
}
```

### Line Counter
```lucid
function count_lines(content: String, count: Int, pos: Int) returns Int {
    return if pos >= content.length() {
        count
    } else {
        // Simplified: just count newlines
        count_lines(content, count + 1, pos + 1)
    }
}

function main() returns Int {
    let content = read_file("input.txt")
    println("File has approximately this many characters:")
    println(content.length())
    return 0
}
```

---

## Success Criteria

Phase 7 is complete when:
- ✅ read_file works for text files
- ✅ write_file creates/overwrites files
- ✅ append_file appends to files
- ✅ file_exists correctly detects files
- ✅ Error cases handled gracefully
- ✅ All tests passing
- ✅ Example programs work

---

## Completion Summary - January 31, 2026

### Implemented Functions
- `read_file(path)` - Returns file contents as string (empty string on error)
- `write_file(path, content)` - Writes/overwrites file, returns true on success
- `append_file(path, content)` - Appends to file, returns true on success
- `file_exists(path)` - Returns true if file exists

### Files Modified
- `include/lucid/backend/bytecode.hpp` - Added BuiltinId entries
- `src/backend/bytecode.cpp` - Added builtin_name entries
- `src/backend/compiler.cpp` - Emit CALL_BUILTIN for file functions
- `src/backend/vm.cpp` - Implemented file I/O using C++ std::fstream
- `src/semantic/type_checker.cpp` - Type signatures for file functions
- `tests/vm_test.cpp` - 7 test cases for file operations

### Test Results
- **7 new test cases**, 17 assertions
- **1020 total assertions** passing (up from 1003)

**Phase 7 Complete!**
