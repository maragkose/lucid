#pragma once

#include <lucid/frontend/token.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace lucid::backend {

// Forward declaration
class Value;

// Bytecode operation codes
enum class OpCode : uint8_t {
    // Literals
    CONSTANT,        // Push constant from constant pool [index: uint16_t]
    TRUE,            // Push true
    FALSE,           // Push false

    // Variables
    LOAD_LOCAL,      // Load local variable by index [index: uint16_t]
    STORE_LOCAL,     // Store to local variable by index [index: uint16_t]
    LOAD_GLOBAL,     // Load global function [index: uint16_t]

    // Arithmetic (binary)
    ADD,             // Add: pop b, pop a, push a + b
    SUB,             // Subtract: pop b, pop a, push a - b
    MUL,             // Multiply: pop b, pop a, push a * b
    DIV,             // Divide: pop b, pop a, push a / b
    MOD,             // Modulo: pop b, pop a, push a % b
    POW,             // Power: pop b, pop a, push a ** b

    // Comparison
    EQ,              // Equal: pop b, pop a, push a == b
    NE,              // Not equal: pop b, pop a, push a != b
    LT,              // Less than: pop b, pop a, push a < b
    GT,              // Greater than: pop b, pop a, push a > b
    LE,              // Less or equal: pop b, pop a, push a <= b
    GE,              // Greater or equal: pop b, pop a, push a >= b

    // Logical
    AND,             // Logical and: pop b, pop a, push a and b
    OR,              // Logical or: pop b, pop a, push a or b
    NOT,             // Logical not: pop a, push not a

    // Unary
    NEGATE,          // Negate: pop a, push -a
    POSITIVE,        // Unary plus: pop a, push +a

    // Collections
    BUILD_LIST,      // Build list from N stack items [count: uint16_t]
    BUILD_TUPLE,     // Build tuple from N stack items [count: uint16_t]
    INDEX,           // Index: pop index, pop object, push object[index]

    // Methods and built-ins
    CALL_METHOD,     // Call built-in method [name_index: uint16_t, arg_count: uint8_t]
    CALL_BUILTIN,    // Call built-in function [builtin_id: uint16_t, arg_count: uint8_t]

    // Control flow
    JUMP,            // Unconditional jump [offset: int16_t]
    JUMP_IF_FALSE,   // Jump if top of stack is false [offset: int16_t]
    JUMP_IF_TRUE,    // Jump if top of stack is true [offset: int16_t]

    // Functions
    CALL,            // Call function [func_index: uint16_t, arg_count: uint8_t]
    RETURN,          // Return from function (value on stack)

    // Stack manipulation
    POP,             // Pop and discard top of stack
    DUP,             // Duplicate top of stack

    // Special
    HALT,            // Stop execution
};

// Built-in function identifiers
enum class BuiltinId : uint16_t {
    PRINT = 0,       // print(value) -> Unit
    PRINTLN = 1,     // println(value) -> Unit
    TO_STRING = 2,   // to_string(value) -> String
    READ_FILE = 3,   // read_file(path: String) -> String
    WRITE_FILE = 4,  // write_file(path: String, content: String) -> Bool
    APPEND_FILE = 5, // append_file(path: String, content: String) -> Bool
    FILE_EXISTS = 6, // file_exists(path: String) -> Bool
};

// Get human-readable name for opcode
auto opcode_name(OpCode opcode) -> std::string_view;

// Get human-readable name for builtin
auto builtin_name(BuiltinId id) -> std::string_view;

// Check if opcode has operands
auto opcode_has_operand(OpCode opcode) -> bool;

// Get operand size in bytes (0, 1, 2, or 3)
auto opcode_operand_size(OpCode opcode) -> size_t;

// Bytecode program structure
class Bytecode {
public:
    Bytecode() = default;

    // Instruction stream
    std::vector<uint8_t> instructions;

    // Constant pool (shared constants)
    std::vector<Value> constants;

    // Function table
    struct FunctionInfo {
        std::string name;
        size_t offset;        // Byte offset in instruction stream
        size_t param_count;   // Number of parameters
        size_t local_count;   // Total local variables (including params)
    };
    std::vector<FunctionInfo> functions;

    // Debug information (optional)
    std::vector<SourceLocation> debug_locations;

    // Bytecode building methods
    auto emit(OpCode opcode) -> void;
    auto emit(OpCode opcode, uint8_t operand) -> void;
    auto emit(OpCode opcode, uint16_t operand) -> void;
    auto emit(OpCode opcode, uint16_t operand1, uint8_t operand2) -> void;

    // Add constant to pool, return index
    auto add_constant(Value value) -> uint16_t;

    // Add function to table
    auto add_function(std::string name, size_t offset, size_t param_count, size_t local_count) -> size_t;

    // Find function by name
    auto find_function(const std::string& name) const -> int;

    // Get current instruction offset
    auto current_offset() const -> size_t { return instructions.size(); }

    // Patch jump instruction at offset
    auto patch_jump(size_t offset, int16_t jump_offset) -> void;

    // Disassemble bytecode for debugging
    auto disassemble(std::string_view name = "Bytecode") const -> std::string;
    auto disassemble_instruction(size_t offset) const -> std::string;

    // Query methods
    auto function_count() const -> size_t { return functions.size(); }
    auto has_function(const std::string& name) const -> bool { return find_function(name) >= 0; }

    // Serialization
    auto save_to_file(const std::string& filename) const -> void;
    static auto load_from_file(const std::string& filename) -> Bytecode;

    // Generate standalone executable source
    auto generate_executable_source() const -> std::string;
};

} // namespace lucid::backend
