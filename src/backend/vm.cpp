#include <lucid/backend/vm.hpp>
#include <fmt/format.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace lucid::backend {

// Constructor
VM::VM() : bytecode_(nullptr) {}

// Main entry point - call a function by name
auto VM::call_function(const Bytecode& bytecode,
                      const std::string& function_name,
                      std::vector<Value> args) -> Value {
    bytecode_ = &bytecode;
    stack_.clear();
    call_stack_.clear();

    // Find function by name
    int func_idx = bytecode.find_function(function_name);
    if (func_idx < 0) {
        throw std::runtime_error(fmt::format("Function '{}' not found", function_name));
    }

    const auto& func_info = bytecode.functions[static_cast<size_t>(func_idx)];

    // Validate argument count
    if (args.size() != func_info.param_count) {
        throw std::runtime_error(fmt::format(
            "Function '{}' expects {} arguments, got {}",
            function_name, func_info.param_count, args.size()
        ));
    }

    // Create initial call frame
    CallFrame frame(
        static_cast<size_t>(func_idx),
        func_info.offset,
        0,  // stack_base
        func_info.local_count
    );

    // Initialize parameters (first N locals) from arguments
    for (size_t i = 0; i < args.size(); ++i) {
        frame.locals[i] = std::move(args[i]);
    }

    call_stack_.push_back(std::move(frame));

    // Execute
    run();

    // Return value should be on stack
    if (stack_.empty()) {
        throw std::runtime_error("Function returned without a value");
    }

    return pop();
}

// Main execution loop
auto VM::run() -> void {
    while (true) {
        uint8_t opcode_byte = read_byte();
        OpCode opcode = static_cast<OpCode>(opcode_byte);

        switch (opcode) {
            // === Literals ===
            case OpCode::CONSTANT: {
                uint16_t idx = read_uint16();
                push(bytecode_->constants[idx]);
                break;
            }

            case OpCode::TRUE:
                push(Value(true));
                break;

            case OpCode::FALSE:
                push(Value(false));
                break;

            // === Arithmetic Operations ===
            case OpCode::ADD: {
                Value b = pop();
                Value a = pop();
                push(binary_add(a, b));
                break;
            }

            case OpCode::SUB: {
                Value b = pop();
                Value a = pop();
                push(binary_sub(a, b));
                break;
            }

            case OpCode::MUL: {
                Value b = pop();
                Value a = pop();
                push(binary_mul(a, b));
                break;
            }

            case OpCode::DIV: {
                Value b = pop();
                Value a = pop();
                push(binary_div(a, b));
                break;
            }

            case OpCode::MOD: {
                Value b = pop();
                Value a = pop();
                push(binary_mod(a, b));
                break;
            }

            case OpCode::POW: {
                Value b = pop();
                Value a = pop();
                push(binary_pow(a, b));
                break;
            }

            // === Comparison Operations ===
            case OpCode::EQ: {
                Value b = pop();
                Value a = pop();
                push(binary_eq(a, b));
                break;
            }

            case OpCode::NE: {
                Value b = pop();
                Value a = pop();
                push(binary_ne(a, b));
                break;
            }

            case OpCode::LT: {
                Value b = pop();
                Value a = pop();
                push(binary_lt(a, b));
                break;
            }

            case OpCode::GT: {
                Value b = pop();
                Value a = pop();
                push(binary_gt(a, b));
                break;
            }

            case OpCode::LE: {
                Value b = pop();
                Value a = pop();
                push(binary_le(a, b));
                break;
            }

            case OpCode::GE: {
                Value b = pop();
                Value a = pop();
                push(binary_ge(a, b));
                break;
            }

            // === Logical Operations ===
            case OpCode::AND: {
                Value b = pop();
                Value a = pop();
                push(binary_and(a, b));
                break;
            }

            case OpCode::OR: {
                Value b = pop();
                Value a = pop();
                push(binary_or(a, b));
                break;
            }

            case OpCode::NOT: {
                Value a = pop();
                push(unary_not(a));
                break;
            }

            // === Unary Operations ===
            case OpCode::NEGATE: {
                Value a = pop();
                push(unary_negate(a));
                break;
            }

            case OpCode::POSITIVE: {
                Value a = pop();
                push(unary_positive(a));
                break;
            }

            // === Stack Operations ===
            case OpCode::POP:
                pop();
                break;

            case OpCode::DUP:
                push(peek());
                break;

            // === Variables ===
            case OpCode::LOAD_LOCAL: {
                uint16_t idx = read_uint16();
                push(current_frame().locals[idx]);
                break;
            }

            case OpCode::STORE_LOCAL: {
                uint16_t idx = read_uint16();
                current_frame().locals[idx] = peek();
                break;
            }

            case OpCode::LOAD_GLOBAL: {
                // Load function reference (push function index as value)
                uint16_t func_idx = read_uint16();
                push(Value(static_cast<int64_t>(func_idx)));
                break;
            }

            // === Function Calls ===
            case OpCode::CALL: {
                // Read operands
                uint16_t func_idx = read_uint16();
                uint8_t arg_count = read_byte();

                // Validate function index
                if (func_idx >= bytecode_->functions.size()) {
                    throw std::runtime_error(fmt::format(
                        "Invalid function index: {}", func_idx
                    ));
                }

                const auto& func_info = bytecode_->functions[func_idx];

                // Validate argument count
                if (arg_count != func_info.param_count) {
                    throw std::runtime_error(fmt::format(
                        "Function '{}' expects {} arguments, got {}",
                        func_info.name, func_info.param_count, arg_count
                    ));
                }

                // Pop arguments from stack (in reverse order)
                std::vector<Value> args;
                args.reserve(arg_count);
                for (size_t i = 0; i < arg_count; ++i) {
                    args.push_back(pop());
                }
                // Reverse to get correct order
                std::reverse(args.begin(), args.end());

                // Create new call frame
                size_t return_address = ip();
                CallFrame new_frame(
                    func_idx,
                    func_info.offset,
                    stack_.size(),  // stack_base for new frame
                    func_info.local_count
                );

                // Initialize parameters from arguments
                for (size_t i = 0; i < args.size(); ++i) {
                    new_frame.locals[i] = std::move(args[i]);
                }

                // Save return address in current frame
                if (!call_stack_.empty()) {
                    current_frame().instruction_pointer = return_address;
                }

                // Push new frame and jump to function
                call_stack_.push_back(std::move(new_frame));
                break;
            }

            case OpCode::RETURN: {
                // Return value should be on top of stack
                // Pop the call frame and return
                call_stack_.pop_back();
                if (call_stack_.empty()) {
                    // Returning from top-level function
                    return;
                }
                // Continue execution in caller
                break;
            }

            // === Control Flow ===
            case OpCode::JUMP: {
                // Read signed 16-bit offset
                int16_t offset = static_cast<int16_t>(read_uint16());
                ip() = static_cast<size_t>(static_cast<int64_t>(ip()) + offset);
                break;
            }

            case OpCode::JUMP_IF_FALSE: {
                // Read signed 16-bit offset
                int16_t offset = static_cast<int16_t>(read_uint16());
                const Value& condition = peek();  // Peek, don't pop
                if (!condition.is_truthy()) {
                    ip() = static_cast<size_t>(static_cast<int64_t>(ip()) + offset);
                }
                break;
            }

            case OpCode::JUMP_IF_TRUE: {
                // Read signed 16-bit offset
                int16_t offset = static_cast<int16_t>(read_uint16());
                const Value& condition = peek();  // Peek, don't pop
                if (condition.is_truthy()) {
                    ip() = static_cast<size_t>(static_cast<int64_t>(ip()) + offset);
                }
                break;
            }

            // === Collections ===
            case OpCode::BUILD_LIST: {
                uint16_t count = read_uint16();
                std::vector<Value> elements;
                elements.reserve(count);

                // Pop elements in reverse order
                for (size_t i = 0; i < count; ++i) {
                    elements.push_back(pop());
                }
                // Reverse to get correct order
                std::reverse(elements.begin(), elements.end());

                push(Value(std::move(elements), false));  // false = list
                break;
            }

            case OpCode::BUILD_TUPLE: {
                uint16_t count = read_uint16();
                std::vector<Value> elements;
                elements.reserve(count);

                // Pop elements in reverse order
                for (size_t i = 0; i < count; ++i) {
                    elements.push_back(pop());
                }
                // Reverse to get correct order
                std::reverse(elements.begin(), elements.end());

                push(Value(std::move(elements), true));  // true = tuple
                break;
            }

            case OpCode::INDEX: {
                Value index = pop();
                Value collection = pop();

                if (!index.is_int()) {
                    throw std::runtime_error(fmt::format(
                        "Index must be Int, got {}",
                        index.type_name()
                    ));
                }

                int64_t idx = index.as_int();

                if (collection.is_list()) {
                    const auto& list = collection.as_list();
                    if (idx < 0 || static_cast<size_t>(idx) >= list.size()) {
                        throw std::runtime_error(fmt::format(
                            "List index out of bounds: {} (size: {})",
                            idx, list.size()
                        ));
                    }
                    push(list[static_cast<size_t>(idx)]);
                } else if (collection.is_tuple()) {
                    const auto& tuple = collection.as_tuple();
                    if (idx < 0 || static_cast<size_t>(idx) >= tuple.size()) {
                        throw std::runtime_error(fmt::format(
                            "Tuple index out of bounds: {} (size: {})",
                            idx, tuple.size()
                        ));
                    }
                    push(tuple[static_cast<size_t>(idx)]);
                } else {
                    throw std::runtime_error(fmt::format(
                        "Cannot index into {}",
                        collection.type_name()
                    ));
                }
                break;
            }

            case OpCode::CALL_METHOD: {
                // Read operands
                uint16_t name_idx = read_uint16();
                uint8_t arg_count = read_byte();

                // Get method name from constant pool
                if (name_idx >= bytecode_->constants.size()) {
                    throw std::runtime_error(fmt::format(
                        "Invalid constant index for method name: {}", name_idx
                    ));
                }
                const Value& name_val = bytecode_->constants[name_idx];
                if (!name_val.is_string()) {
                    throw std::runtime_error("Method name must be a string");
                }
                const std::string& method_name = name_val.as_string();

                // Pop arguments (in reverse order)
                std::vector<Value> args;
                args.reserve(arg_count);
                for (size_t i = 0; i < arg_count; ++i) {
                    args.push_back(pop());
                }
                std::reverse(args.begin(), args.end());

                // Pop the receiver object
                Value receiver = pop();

                // Dispatch to built-in method
                Value result = call_builtin_method(method_name, receiver, std::move(args));
                push(std::move(result));
                break;
            }

            case OpCode::CALL_BUILTIN: {
                // Read operands
                uint16_t builtin_id = read_uint16();
                uint8_t arg_count = read_byte();

                // Pop arguments (in reverse order)
                std::vector<Value> args;
                args.reserve(arg_count);
                for (size_t i = 0; i < arg_count; ++i) {
                    args.push_back(pop());
                }
                std::reverse(args.begin(), args.end());

                // Dispatch based on builtin ID
                switch (static_cast<BuiltinId>(builtin_id)) {
                    case BuiltinId::PRINT: {
                        if (args.size() != 1) {
                            throw std::runtime_error("print() expects 1 argument");
                        }
                        // Print without newline - strings printed without quotes
                        if (args[0].is_string()) {
                            output_stream_ << args[0].as_string();
                        } else {
                            output_stream_ << args[0].to_string();
                        }
                        // Return Unit (push nothing or a placeholder)
                        push(Value(int64_t{0}));  // Unit placeholder
                        break;
                    }
                    case BuiltinId::PRINTLN: {
                        if (args.size() != 1) {
                            throw std::runtime_error("println() expects 1 argument");
                        }
                        // Print with newline - strings printed without quotes
                        if (args[0].is_string()) {
                            output_stream_ << args[0].as_string() << "\n";
                        } else {
                            output_stream_ << args[0].to_string() << "\n";
                        }
                        // Return Unit (push nothing or a placeholder)
                        push(Value(int64_t{0}));  // Unit placeholder
                        break;
                    }
                    case BuiltinId::TO_STRING: {
                        if (args.size() != 1) {
                            throw std::runtime_error("to_string() expects 1 argument");
                        }
                        push(Value(args[0].to_string()));
                        break;
                    }
                    case BuiltinId::READ_FILE: {
                        if (args.size() != 1 || !args[0].is_string()) {
                            throw std::runtime_error("read_file() expects 1 string argument");
                        }
                        const std::string& path = args[0].as_string();
                        std::ifstream file(path);
                        if (!file.is_open()) {
                            // Return empty string on error
                            push(Value(std::string{}));
                        } else {
                            std::stringstream buffer;
                            buffer << file.rdbuf();
                            push(Value(buffer.str()));
                        }
                        break;
                    }
                    case BuiltinId::WRITE_FILE: {
                        if (args.size() != 2 || !args[0].is_string() || !args[1].is_string()) {
                            throw std::runtime_error("write_file() expects 2 string arguments");
                        }
                        const std::string& path = args[0].as_string();
                        const std::string& content = args[1].as_string();
                        std::ofstream file(path);
                        if (!file.is_open()) {
                            push(Value(false));
                        } else {
                            file << content;
                            push(Value(file.good()));
                        }
                        break;
                    }
                    case BuiltinId::APPEND_FILE: {
                        if (args.size() != 2 || !args[0].is_string() || !args[1].is_string()) {
                            throw std::runtime_error("append_file() expects 2 string arguments");
                        }
                        const std::string& path = args[0].as_string();
                        const std::string& content = args[1].as_string();
                        std::ofstream file(path, std::ios::app);
                        if (!file.is_open()) {
                            push(Value(false));
                        } else {
                            file << content;
                            push(Value(file.good()));
                        }
                        break;
                    }
                    case BuiltinId::FILE_EXISTS: {
                        if (args.size() != 1 || !args[0].is_string()) {
                            throw std::runtime_error("file_exists() expects 1 string argument");
                        }
                        const std::string& path = args[0].as_string();
                        push(Value(std::filesystem::exists(path)));
                        break;
                    }
                    default:
                        throw std::runtime_error(fmt::format(
                            "Unknown builtin ID: {}", builtin_id
                        ));
                }
                break;
            }

            case OpCode::HALT:
                return;

            default:
                throw std::runtime_error(fmt::format(
                    "Unknown opcode: {}",
                    static_cast<int>(opcode_byte)
                ));
        }
    }
}

// === Helper Methods ===

auto VM::current_frame() -> CallFrame& {
    return call_stack_.back();
}

auto VM::ip() -> size_t& {
    return current_frame().instruction_pointer;
}

auto VM::push(Value val) -> void {
    stack_.push_back(std::move(val));
}

auto VM::pop() -> Value {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value val = std::move(stack_.back());
    stack_.pop_back();
    return val;
}

auto VM::peek() const -> const Value& {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    return stack_.back();
}

auto VM::read_byte() -> uint8_t {
    return bytecode_->instructions[ip()++];
}

auto VM::read_uint16() -> uint16_t {
    // Little-endian
    uint8_t low = read_byte();
    uint8_t high = read_byte();
    uint16_t result = static_cast<uint16_t>(static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8));
    return result;
}

// === Arithmetic Operations ===

auto VM::binary_add(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        return Value(a.as_int() + b.as_int());
    }
    if (a.is_float() && b.is_float()) {
        return Value(a.as_float() + b.as_float());
    }
    if (a.is_int() && b.is_float()) {
        return Value(static_cast<double>(a.as_int()) + b.as_float());
    }
    if (a.is_float() && b.is_int()) {
        return Value(a.as_float() + static_cast<double>(b.as_int()));
    }
    throw std::runtime_error(fmt::format(
        "Cannot add {} and {}",
        a.type_name(), b.type_name()
    ));
}

auto VM::binary_sub(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        return Value(a.as_int() - b.as_int());
    }
    if (a.is_float() && b.is_float()) {
        return Value(a.as_float() - b.as_float());
    }
    if (a.is_int() && b.is_float()) {
        return Value(static_cast<double>(a.as_int()) - b.as_float());
    }
    if (a.is_float() && b.is_int()) {
        return Value(a.as_float() - static_cast<double>(b.as_int()));
    }
    throw std::runtime_error(fmt::format(
        "Cannot subtract {} and {}",
        a.type_name(), b.type_name()
    ));
}

auto VM::binary_mul(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        return Value(a.as_int() * b.as_int());
    }
    if (a.is_float() && b.is_float()) {
        return Value(a.as_float() * b.as_float());
    }
    if (a.is_int() && b.is_float()) {
        return Value(static_cast<double>(a.as_int()) * b.as_float());
    }
    if (a.is_float() && b.is_int()) {
        return Value(a.as_float() * static_cast<double>(b.as_int()));
    }
    throw std::runtime_error(fmt::format(
        "Cannot multiply {} and {}",
        a.type_name(), b.type_name()
    ));
}

auto VM::binary_div(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        if (b.as_int() == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Value(a.as_int() / b.as_int());
    }
    if (a.is_float() && b.is_float()) {
        if (b.as_float() == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        return Value(a.as_float() / b.as_float());
    }
    if (a.is_int() && b.is_float()) {
        if (b.as_float() == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        return Value(static_cast<double>(a.as_int()) / b.as_float());
    }
    if (a.is_float() && b.is_int()) {
        if (b.as_int() == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Value(a.as_float() / static_cast<double>(b.as_int()));
    }
    throw std::runtime_error(fmt::format(
        "Cannot divide {} and {}",
        a.type_name(), b.type_name()
    ));
}

auto VM::binary_mod(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        if (b.as_int() == 0) {
            throw std::runtime_error("Modulo by zero");
        }
        return Value(a.as_int() % b.as_int());
    }
    throw std::runtime_error(fmt::format(
        "Modulo requires two integers, got {} and {}",
        a.type_name(), b.type_name()
    ));
}

auto VM::binary_pow(const Value& a, const Value& b) -> Value {
    if (a.is_int() && b.is_int()) {
        return Value(static_cast<int64_t>(std::pow(a.as_int(), b.as_int())));
    }
    if (a.is_float() && b.is_float()) {
        return Value(std::pow(a.as_float(), b.as_float()));
    }
    if (a.is_int() && b.is_float()) {
        return Value(std::pow(static_cast<double>(a.as_int()), b.as_float()));
    }
    if (a.is_float() && b.is_int()) {
        return Value(std::pow(a.as_float(), static_cast<double>(b.as_int())));
    }
    throw std::runtime_error(fmt::format(
        "Cannot raise {} to power of {}",
        a.type_name(), b.type_name()
    ));
}

// === Comparison Operations ===

auto VM::binary_eq(const Value& a, const Value& b) -> Value {
    return Value(a == b);
}

auto VM::binary_ne(const Value& a, const Value& b) -> Value {
    return Value(a != b);
}

auto VM::binary_lt(const Value& a, const Value& b) -> Value {
    return Value(a < b);
}

auto VM::binary_gt(const Value& a, const Value& b) -> Value {
    return Value(a > b);
}

auto VM::binary_le(const Value& a, const Value& b) -> Value {
    return Value(a <= b);
}

auto VM::binary_ge(const Value& a, const Value& b) -> Value {
    return Value(a >= b);
}

// === Logical Operations ===

auto VM::binary_and(const Value& a, const Value& b) -> Value {
    return Value(a.is_truthy() && b.is_truthy());
}

auto VM::binary_or(const Value& a, const Value& b) -> Value {
    return Value(a.is_truthy() || b.is_truthy());
}

auto VM::unary_not(const Value& a) -> Value {
    return Value(!a.is_truthy());
}

// === Unary Operations ===

auto VM::unary_negate(const Value& a) -> Value {
    if (a.is_int()) {
        return Value(-a.as_int());
    }
    if (a.is_float()) {
        return Value(-a.as_float());
    }
    throw std::runtime_error(fmt::format(
        "Cannot negate {}",
        a.type_name()
    ));
}

auto VM::unary_positive(const Value& a) -> Value {
    if (a.is_int() || a.is_float()) {
        return a;  // Unary + just returns the value
    }
    throw std::runtime_error(fmt::format(
        "Cannot apply unary + to {}",
        a.type_name()
    ));
}

// === Built-in Methods ===

auto VM::call_builtin_method(const std::string& method,
                             Value& object,
                             std::vector<Value> args) -> Value {
    // List methods
    if (object.is_list()) {
        if (method == "length") {
            if (!args.empty()) {
                throw std::runtime_error("List.length() takes no arguments");
            }
            return Value(static_cast<int64_t>(object.as_list().size()));
        }
        if (method == "append") {
            if (args.size() != 1) {
                throw std::runtime_error("List.append() takes exactly 1 argument");
            }
            // Create a new list with the element appended (immutable)
            std::vector<Value> new_list = object.as_list();
            new_list.push_back(std::move(args[0]));
            return Value(std::move(new_list), false);  // false = list
        }
        if (method == "head") {
            if (!args.empty()) {
                throw std::runtime_error("List.head() takes no arguments");
            }
            const auto& list = object.as_list();
            if (list.empty()) {
                throw std::runtime_error("List.head() on empty list");
            }
            return list[0];
        }
        if (method == "tail") {
            if (!args.empty()) {
                throw std::runtime_error("List.tail() takes no arguments");
            }
            const auto& list = object.as_list();
            if (list.empty()) {
                throw std::runtime_error("List.tail() on empty list");
            }
            std::vector<Value> new_list(list.begin() + 1, list.end());
            return Value(std::move(new_list), false);  // false = list
        }
        if (method == "is_empty") {
            if (!args.empty()) {
                throw std::runtime_error("List.is_empty() takes no arguments");
            }
            return Value(object.as_list().empty());
        }
        if (method == "reverse") {
            if (!args.empty()) {
                throw std::runtime_error("List.reverse() takes no arguments");
            }
            std::vector<Value> new_list = object.as_list();
            std::reverse(new_list.begin(), new_list.end());
            return Value(std::move(new_list), false);  // false = list
        }
        if (method == "concat") {
            if (args.size() != 1 || !args[0].is_list()) {
                throw std::runtime_error("List.concat() takes 1 list argument");
            }
            std::vector<Value> new_list = object.as_list();
            const auto& other = args[0].as_list();
            new_list.insert(new_list.end(), other.begin(), other.end());
            return Value(std::move(new_list), false);  // false = list
        }
        throw std::runtime_error(fmt::format(
            "Unknown method '{}' on List", method
        ));
    }

    // Tuple methods
    if (object.is_tuple()) {
        if (method == "length") {
            if (!args.empty()) {
                throw std::runtime_error("Tuple.length() takes no arguments");
            }
            return Value(static_cast<int64_t>(object.as_tuple().size()));
        }
        throw std::runtime_error(fmt::format(
            "Unknown method '{}' on Tuple", method
        ));
    }

    // String methods
    if (object.is_string()) {
        if (method == "length") {
            if (!args.empty()) {
                throw std::runtime_error("String.length() takes no arguments");
            }
            return Value(static_cast<int64_t>(object.as_string().size()));
        }
        if (method == "is_empty") {
            if (!args.empty()) {
                throw std::runtime_error("String.is_empty() takes no arguments");
            }
            return Value(object.as_string().empty());
        }
        if (method == "contains") {
            if (args.size() != 1 || !args[0].is_string()) {
                throw std::runtime_error("String.contains() takes 1 string argument");
            }
            return Value(object.as_string().find(args[0].as_string()) != std::string::npos);
        }
        if (method == "starts_with") {
            if (args.size() != 1 || !args[0].is_string()) {
                throw std::runtime_error("String.starts_with() takes 1 string argument");
            }
            const auto& str = object.as_string();
            const auto& prefix = args[0].as_string();
            return Value(str.size() >= prefix.size() &&
                        str.compare(0, prefix.size(), prefix) == 0);
        }
        if (method == "ends_with") {
            if (args.size() != 1 || !args[0].is_string()) {
                throw std::runtime_error("String.ends_with() takes 1 string argument");
            }
            const auto& str = object.as_string();
            const auto& suffix = args[0].as_string();
            return Value(str.size() >= suffix.size() &&
                        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
        }
        if (method == "to_upper") {
            if (!args.empty()) {
                throw std::runtime_error("String.to_upper() takes no arguments");
            }
            std::string result = object.as_string();
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::toupper(c); });
            return Value(std::move(result));
        }
        if (method == "to_lower") {
            if (!args.empty()) {
                throw std::runtime_error("String.to_lower() takes no arguments");
            }
            std::string result = object.as_string();
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            return Value(std::move(result));
        }
        if (method == "trim") {
            if (!args.empty()) {
                throw std::runtime_error("String.trim() takes no arguments");
            }
            std::string result = object.as_string();
            // Trim leading whitespace
            auto start = result.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) {
                return Value(std::string{});
            }
            // Trim trailing whitespace
            auto end = result.find_last_not_of(" \t\n\r");
            return Value(result.substr(start, end - start + 1));
        }
        throw std::runtime_error(fmt::format(
            "Unknown method '{}' on String", method
        ));
    }

    // Int methods
    if (object.is_int()) {
        if (method == "to_string") {
            if (!args.empty()) {
                throw std::runtime_error("Int.to_string() takes no arguments");
            }
            return Value(std::to_string(object.as_int()));
        }
        if (method == "abs") {
            if (!args.empty()) {
                throw std::runtime_error("Int.abs() takes no arguments");
            }
            int64_t val = object.as_int();
            return Value(val < 0 ? -val : val);
        }
        throw std::runtime_error(fmt::format(
            "Unknown method '{}' on Int", method
        ));
    }

    // Float methods
    if (object.is_float()) {
        if (method == "to_string") {
            if (!args.empty()) {
                throw std::runtime_error("Float.to_string() takes no arguments");
            }
            return Value(fmt::format("{}", object.as_float()));
        }
        if (method == "abs") {
            if (!args.empty()) {
                throw std::runtime_error("Float.abs() takes no arguments");
            }
            return Value(std::abs(object.as_float()));
        }
        if (method == "floor") {
            if (!args.empty()) {
                throw std::runtime_error("Float.floor() takes no arguments");
            }
            return Value(static_cast<int64_t>(std::floor(object.as_float())));
        }
        if (method == "ceil") {
            if (!args.empty()) {
                throw std::runtime_error("Float.ceil() takes no arguments");
            }
            return Value(static_cast<int64_t>(std::ceil(object.as_float())));
        }
        if (method == "round") {
            if (!args.empty()) {
                throw std::runtime_error("Float.round() takes no arguments");
            }
            return Value(static_cast<int64_t>(std::round(object.as_float())));
        }
        throw std::runtime_error(fmt::format(
            "Unknown method '{}' on Float", method
        ));
    }

    throw std::runtime_error(fmt::format(
        "Cannot call method '{}' on {}",
        method, object.type_name()
    ));
}

} // namespace lucid::backend
