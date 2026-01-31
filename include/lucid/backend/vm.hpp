#pragma once

#include <lucid/backend/bytecode.hpp>
#include <lucid/backend/value.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace lucid::backend {

// Call frame for function invocations
struct CallFrame {
    size_t function_index;        // Index in bytecode function table
    size_t instruction_pointer;   // Current instruction offset in bytecode
    size_t stack_base;            // Base of this frame's operands in stack
    std::vector<Value> locals;    // Local variable storage

    CallFrame(size_t func_idx, size_t ip, size_t base, size_t local_count)
        : function_index(func_idx)
        , instruction_pointer(ip)
        , stack_base(base)
        , locals(local_count, Value(int64_t{0}))  // Initialize locals to 0
    {}
};

// Stack-based virtual machine for bytecode execution
class VM {
public:
    VM();

    /**
     * Execute a specific function by name with arguments.
     * This is the main entry point for execution.
     *
     * @param bytecode The bytecode to execute
     * @param function_name Name of function to call
     * @param args Arguments to pass to the function
     * @return Result value from function
     * @throws std::runtime_error on execution errors
     */
    auto call_function(const Bytecode& bytecode,
                      const std::string& function_name,
                      std::vector<Value> args) -> Value;

    /**
     * Set custom output stream for print/println.
     * Defaults to std::cout.
     */
    auto set_output_stream(std::ostream& os) -> void { output_stream_.rdbuf(os.rdbuf()); }

    /**
     * Get captured output (for testing).
     * Only works if a stringstream was set as output.
     */
    auto get_output() const -> std::string { return output_buffer_.str(); }

    /**
     * Clear captured output buffer.
     */
    auto clear_output() -> void { output_buffer_.str(""); output_buffer_.clear(); }

    /**
     * Use internal buffer for output (for testing).
     */
    auto use_output_buffer() -> void { output_stream_.rdbuf(output_buffer_.rdbuf()); }

private:
    // Execution state
    const Bytecode* bytecode_;
    std::vector<Value> stack_;        // Operand stack
    std::vector<CallFrame> call_stack_;  // Call frames

    // Output stream for print/println (defaults to cout)
    std::ostream output_stream_{std::cout.rdbuf()};
    std::stringstream output_buffer_;  // For testing

    // Main execution loop
    auto run() -> void;

    // Current frame accessors
    auto current_frame() -> CallFrame&;
    auto ip() -> size_t&;  // Instruction pointer reference

    // Stack operations
    auto push(Value val) -> void;
    auto pop() -> Value;
    auto peek() const -> const Value&;

    // Instruction reading
    auto read_byte() -> uint8_t;
    auto read_uint16() -> uint16_t;

    // Arithmetic operations
    auto binary_add(const Value& a, const Value& b) -> Value;
    auto binary_sub(const Value& a, const Value& b) -> Value;
    auto binary_mul(const Value& a, const Value& b) -> Value;
    auto binary_div(const Value& a, const Value& b) -> Value;
    auto binary_mod(const Value& a, const Value& b) -> Value;
    auto binary_pow(const Value& a, const Value& b) -> Value;

    // Comparison operations
    auto binary_eq(const Value& a, const Value& b) -> Value;
    auto binary_ne(const Value& a, const Value& b) -> Value;
    auto binary_lt(const Value& a, const Value& b) -> Value;
    auto binary_gt(const Value& a, const Value& b) -> Value;
    auto binary_le(const Value& a, const Value& b) -> Value;
    auto binary_ge(const Value& a, const Value& b) -> Value;

    // Logical operations
    auto binary_and(const Value& a, const Value& b) -> Value;
    auto binary_or(const Value& a, const Value& b) -> Value;
    auto unary_not(const Value& a) -> Value;

    // Unary operations
    auto unary_negate(const Value& a) -> Value;
    auto unary_positive(const Value& a) -> Value;

    // Built-in methods
    auto call_builtin_method(const std::string& method,
                            Value& object,
                            std::vector<Value> args) -> Value;
};

} // namespace lucid::backend
