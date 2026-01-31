#include <lucid/backend/bytecode.hpp>
#include <lucid/backend/value.hpp>
#include <fmt/format.h>
#include <stdexcept>

namespace lucid::backend {

// Get human-readable name for opcode
auto opcode_name(OpCode opcode) -> std::string_view {
    switch (opcode) {
        case OpCode::CONSTANT: return "CONSTANT";
        case OpCode::TRUE: return "TRUE";
        case OpCode::FALSE: return "FALSE";
        case OpCode::LOAD_LOCAL: return "LOAD_LOCAL";
        case OpCode::STORE_LOCAL: return "STORE_LOCAL";
        case OpCode::LOAD_GLOBAL: return "LOAD_GLOBAL";
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::MOD: return "MOD";
        case OpCode::POW: return "POW";
        case OpCode::EQ: return "EQ";
        case OpCode::NE: return "NE";
        case OpCode::LT: return "LT";
        case OpCode::GT: return "GT";
        case OpCode::LE: return "LE";
        case OpCode::GE: return "GE";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        case OpCode::NEGATE: return "NEGATE";
        case OpCode::POSITIVE: return "POSITIVE";
        case OpCode::BUILD_LIST: return "BUILD_LIST";
        case OpCode::BUILD_TUPLE: return "BUILD_TUPLE";
        case OpCode::INDEX: return "INDEX";
        case OpCode::CALL_METHOD: return "CALL_METHOD";
        case OpCode::CALL_BUILTIN: return "CALL_BUILTIN";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case OpCode::CALL: return "CALL";
        case OpCode::RETURN: return "RETURN";
        case OpCode::POP: return "POP";
        case OpCode::DUP: return "DUP";
        case OpCode::HALT: return "HALT";
    }
    return "UNKNOWN";
}

// Get human-readable name for builtin
auto builtin_name(BuiltinId id) -> std::string_view {
    switch (id) {
        case BuiltinId::PRINT: return "print";
        case BuiltinId::PRINTLN: return "println";
        case BuiltinId::TO_STRING: return "to_string";
        case BuiltinId::READ_FILE: return "read_file";
        case BuiltinId::WRITE_FILE: return "write_file";
        case BuiltinId::APPEND_FILE: return "append_file";
        case BuiltinId::FILE_EXISTS: return "file_exists";
    }
    return "UNKNOWN_BUILTIN";
}

// Check if opcode has operands
auto opcode_has_operand(OpCode opcode) -> bool {
    return opcode_operand_size(opcode) > 0;
}

// Get operand size in bytes
auto opcode_operand_size(OpCode opcode) -> size_t {
    switch (opcode) {
        // No operands
        case OpCode::TRUE:
        case OpCode::FALSE:
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::MOD:
        case OpCode::POW:
        case OpCode::EQ:
        case OpCode::NE:
        case OpCode::LT:
        case OpCode::GT:
        case OpCode::LE:
        case OpCode::GE:
        case OpCode::AND:
        case OpCode::OR:
        case OpCode::NOT:
        case OpCode::NEGATE:
        case OpCode::POSITIVE:
        case OpCode::INDEX:
        case OpCode::RETURN:
        case OpCode::POP:
        case OpCode::DUP:
        case OpCode::HALT:
            return 0;

        // 2-byte operand (uint16_t)
        case OpCode::CONSTANT:
        case OpCode::LOAD_LOCAL:
        case OpCode::STORE_LOCAL:
        case OpCode::LOAD_GLOBAL:
        case OpCode::BUILD_LIST:
        case OpCode::BUILD_TUPLE:
        case OpCode::JUMP:
        case OpCode::JUMP_IF_FALSE:
        case OpCode::JUMP_IF_TRUE:
            return 2;

        // 3-byte operand (uint16_t + uint8_t)
        case OpCode::CALL_METHOD:
        case OpCode::CALL_BUILTIN:
        case OpCode::CALL:
            return 3;
    }
    return 0;
}

// Bytecode building methods
auto Bytecode::emit(OpCode opcode) -> void {
    instructions.push_back(static_cast<uint8_t>(opcode));
}

auto Bytecode::emit(OpCode opcode, uint8_t operand) -> void {
    instructions.push_back(static_cast<uint8_t>(opcode));
    instructions.push_back(operand);
}

auto Bytecode::emit(OpCode opcode, uint16_t operand) -> void {
    instructions.push_back(static_cast<uint8_t>(opcode));
    // Little-endian encoding
    instructions.push_back(static_cast<uint8_t>(operand & 0xFF));
    instructions.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
}

auto Bytecode::emit(OpCode opcode, uint16_t operand1, uint8_t operand2) -> void {
    instructions.push_back(static_cast<uint8_t>(opcode));
    // Little-endian encoding for operand1
    instructions.push_back(static_cast<uint8_t>(operand1 & 0xFF));
    instructions.push_back(static_cast<uint8_t>((operand1 >> 8) & 0xFF));
    instructions.push_back(operand2);
}

// Add constant to pool
auto Bytecode::add_constant(Value value) -> uint16_t {
    constants.push_back(std::move(value));
    if (constants.size() > UINT16_MAX) {
        throw std::runtime_error("Too many constants");
    }
    return static_cast<uint16_t>(constants.size() - 1);
}

// Add function to table
auto Bytecode::add_function(std::string name, size_t offset, size_t param_count, size_t local_count) -> size_t {
    functions.push_back({std::move(name), offset, param_count, local_count});
    return functions.size() - 1;
}

// Find function by name
auto Bytecode::find_function(const std::string& name) const -> int {
    for (size_t i = 0; i < functions.size(); ++i) {
        if (functions[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Patch jump instruction
auto Bytecode::patch_jump(size_t offset, int16_t jump_offset) -> void {
    if (offset + 2 >= instructions.size()) {
        throw std::runtime_error("Invalid jump patch offset");
    }

    // Write jump offset as little-endian int16_t
    instructions[offset + 1] = static_cast<uint8_t>(jump_offset & 0xFF);
    instructions[offset + 2] = static_cast<uint8_t>((jump_offset >> 8) & 0xFF);
}

// Disassemble bytecode
auto Bytecode::disassemble(std::string_view name) const -> std::string {
    std::string result = fmt::format("== {} ==\n", name);

    // Print constants
    if (!constants.empty()) {
        result += "\n=== Constants ===\n";
        for (size_t i = 0; i < constants.size(); ++i) {
            result += fmt::format("  [{}] {}\n", i, constants[i].to_string());
        }
    }

    // Print functions
    if (!functions.empty()) {
        result += "\n=== Functions ===\n";
        for (size_t i = 0; i < functions.size(); ++i) {
            const auto& func = functions[i];
            result += fmt::format("  [{}] {} (offset: {}, params: {}, locals: {})\n",
                                  i, func.name, func.offset, func.param_count, func.local_count);
        }
    }

    // Print instructions
    result += "\n=== Instructions ===\n";
    size_t offset = 0;
    while (offset < instructions.size()) {
        auto line = disassemble_instruction(offset);
        result += line + "\n";

        // Advance offset
        auto opcode = static_cast<OpCode>(instructions[offset]);
        offset += 1 + opcode_operand_size(opcode);
    }

    return result;
}

// Disassemble single instruction
auto Bytecode::disassemble_instruction(size_t offset) const -> std::string {
    if (offset >= instructions.size()) {
        return fmt::format("{:04d}  ERROR: offset out of bounds", offset);
    }

    auto opcode = static_cast<OpCode>(instructions[offset]);
    auto name = opcode_name(opcode);

    // Check if this is a function entry point
    std::string func_label;
    for (const auto& func : functions) {
        if (func.offset == offset) {
            func_label = fmt::format("\n--- {} ---\n", func.name);
            break;
        }
    }

    // Format based on operand size
    size_t operand_size = opcode_operand_size(opcode);

    if (operand_size == 0) {
        return fmt::format("{}{:04d}  {}", func_label, offset, name);
    }

    if (operand_size == 2) {
        if (offset + 2 >= instructions.size()) {
            return fmt::format("{}{:04d}  {} ERROR: incomplete operand", func_label, offset, name);
        }

        // Read uint16_t operand (little-endian)
        uint16_t operand = static_cast<uint16_t>(
            static_cast<uint16_t>(instructions[offset + 1]) |
            (static_cast<uint16_t>(instructions[offset + 2]) << 8)
        );

        // Special formatting for specific opcodes
        if (opcode == OpCode::CONSTANT && operand < constants.size()) {
            return fmt::format("{}{:04d}  {} {} ({})",
                               func_label, offset, name, operand, constants[operand].to_string());
        } else if (opcode == OpCode::JUMP || opcode == OpCode::JUMP_IF_FALSE || opcode == OpCode::JUMP_IF_TRUE) {
            // Interpret as signed offset
            int16_t signed_offset = static_cast<int16_t>(operand);
            int64_t target = static_cast<int64_t>(offset) + 3 + signed_offset;
            return fmt::format("{}{:04d}  {} {} (to {})",
                               func_label, offset, name, signed_offset, target);
        } else {
            return fmt::format("{}{:04d}  {} {}", func_label, offset, name, operand);
        }
    }

    if (operand_size == 3) {
        if (offset + 3 >= instructions.size()) {
            return fmt::format("{}{:04d}  {} ERROR: incomplete operand", func_label, offset, name);
        }

        // Read uint16_t + uint8_t operands
        uint16_t operand1 = static_cast<uint16_t>(
            static_cast<uint16_t>(instructions[offset + 1]) |
            (static_cast<uint16_t>(instructions[offset + 2]) << 8)
        );
        uint8_t operand2 = instructions[offset + 3];

        if (opcode == OpCode::CALL_METHOD && operand1 < constants.size()) {
            return fmt::format("{}{:04d}  {} {} ({}), args: {}",
                               func_label, offset, name, operand1, constants[operand1].to_string(), operand2);
        } else if (opcode == OpCode::CALL_BUILTIN) {
            return fmt::format("{}{:04d}  {} {} ({}), args: {}",
                               func_label, offset, name, operand1, builtin_name(static_cast<BuiltinId>(operand1)), operand2);
        } else if (opcode == OpCode::CALL && operand1 < functions.size()) {
            return fmt::format("{}{:04d}  {} {} ({}), args: {}",
                               func_label, offset, name, operand1, functions[operand1].name, operand2);
        } else {
            return fmt::format("{}{:04d}  {} {}, {}", func_label, offset, name, operand1, operand2);
        }
    }

    return fmt::format("{}{:04d}  {} UNKNOWN_OPERAND", func_label, offset, name);
}

// Generate C++ source code with embedded bytecode
auto Bytecode::generate_executable_source() const -> std::string {
    std::string source;

    // Header
    source += "#include <lucid/backend/vm.hpp>\n";
    source += "#include <lucid/backend/value.hpp>\n";
    source += "#include <lucid/backend/bytecode.hpp>\n";
    source += "#include <vector>\n";
    source += "#include <string>\n";
    source += "#include <cstdint>\n";
    source += "#include <iostream>\n\n";

    source += "// Auto-generated LUCID bytecode executable\n\n";

    // Embedded bytecode instructions
    source += "static const uint8_t BYTECODE_INSTRUCTIONS[] = {\n    ";
    for (size_t i = 0; i < instructions.size(); ++i) {
        source += fmt::format("0x{:02x}", instructions[i]);
        if (i < instructions.size() - 1) {
            source += ", ";
            if ((i + 1) % 16 == 0) source += "\n    ";
        }
    }
    source += "\n};\n\n";

    // Embedded constants
    source += "static void init_constants(lucid::backend::Bytecode& bc) {\n";
    for (const auto& constant : constants) {
        if (constant.is_int()) {
            source += fmt::format("    bc.add_constant(lucid::backend::Value(int64_t{{{}}}));\n",
                                constant.as_int());
        } else if (constant.is_float()) {
            source += fmt::format("    bc.add_constant(lucid::backend::Value(double{{{}}}));\n",
                                constant.as_float());
        } else if (constant.is_string()) {
            // Escape string properly
            std::string escaped;
            for (char c : constant.as_string()) {
                if (c == '"') escaped += "\\\"";
                else if (c == '\\') escaped += "\\\\";
                else if (c == '\n') escaped += "\\n";
                else if (c == '\t') escaped += "\\t";
                else if (c == '\r') escaped += "\\r";
                else escaped += c;
            }
            source += fmt::format("    bc.add_constant(lucid::backend::Value(std::string{{\"{}\"}}));\n",
                                escaped);
        } else if (constant.is_bool()) {
            source += fmt::format("    bc.add_constant(lucid::backend::Value({}));\n",
                                constant.as_bool() ? "true" : "false");
        }
    }
    source += "}\n\n";

    // Embedded functions
    source += "static void init_functions(lucid::backend::Bytecode& bc) {\n";
    for (const auto& func : functions) {
        // Escape function name
        std::string escaped_name;
        for (char c : func.name) {
            if (c == '"') escaped_name += "\\\"";
            else if (c == '\\') escaped_name += "\\\\";
            else escaped_name += c;
        }
        source += fmt::format("    bc.add_function(\"{}\", {}, {}, {});\n",
                            escaped_name, func.offset, func.param_count, func.local_count);
    }
    source += "}\n\n";

    // Main function
    source += "int main() {\n";
    source += "    try {\n";
    source += "        // Reconstruct bytecode\n";
    source += "        lucid::backend::Bytecode bytecode;\n";
    source += "        bytecode.instructions.assign(BYTECODE_INSTRUCTIONS, \n";
    source += "                                    BYTECODE_INSTRUCTIONS + sizeof(BYTECODE_INSTRUCTIONS));\n";
    source += "        init_constants(bytecode);\n";
    source += "        init_functions(bytecode);\n\n";
    source += "        // Execute main function\n";
    source += "        lucid::backend::VM vm;\n";
    source += "        auto result = vm.call_function(bytecode, \"main\", {});\n\n";
    source += "        // Return exit code\n";
    source += "        if (result.is_int()) {\n";
    source += "            return static_cast<int>(result.as_int());\n";
    source += "        }\n";
    source += "        return 0;\n";
    source += "    } catch (const std::exception& e) {\n";
    source += "        std::cerr << \"Runtime error: \" << e.what() << std::endl;\n";
    source += "        return 1;\n";
    source += "    }\n";
    source += "}\n";

    return source;
}

} // namespace lucid::backend
