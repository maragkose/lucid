// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include "catch_amalgamated.hpp"
#endif

#include <lucid/backend/value.hpp>
#include <lucid/backend/bytecode.hpp>
#include <lucid/backend/compiler.hpp>
#include <lucid/frontend/ast.hpp>

using namespace lucid::backend;

// ===== Value Tests =====

TEST_CASE("Value: Int construction and type checking", "[compiler][value]") {
    Value val(int64_t{42});

    REQUIRE(val.is_int());
    REQUIRE(!val.is_float());
    REQUIRE(!val.is_bool());
    REQUIRE(!val.is_string());
    REQUIRE(!val.is_list());
    REQUIRE(!val.is_tuple());

    REQUIRE(val.as_int() == 42);
    REQUIRE(val.type_name() == "Int");
}

TEST_CASE("Value: Float construction and type checking", "[compiler][value]") {
    Value val(3.14);

    REQUIRE(val.is_float());
    REQUIRE(!val.is_int());

    REQUIRE(val.as_float() == 3.14);
    REQUIRE(val.type_name() == "Float");
}

TEST_CASE("Value: Bool construction and type checking", "[compiler][value]") {
    Value val_true(true);
    Value val_false(false);

    REQUIRE(val_true.is_bool());
    REQUIRE(val_true.as_bool() == true);

    REQUIRE(val_false.is_bool());
    REQUIRE(val_false.as_bool() == false);

    REQUIRE(val_true.type_name() == "Bool");
}

TEST_CASE("Value: String construction and type checking", "[compiler][value]") {
    Value val(std::string("hello"));

    REQUIRE(val.is_string());
    REQUIRE(val.as_string() == "hello");
    REQUIRE(val.type_name() == "String");
}

TEST_CASE("Value: List construction and type checking", "[compiler][value]") {
    std::vector<Value> elements;
    elements.push_back(Value(int64_t{1}));
    elements.push_back(Value(int64_t{2}));
    elements.push_back(Value(int64_t{3}));

    Value val(std::move(elements), false);  // false = list, not tuple

    REQUIRE(val.is_list());
    REQUIRE(!val.is_tuple());
    REQUIRE(val.as_list().size() == 3);
    REQUIRE(val.as_list()[0].as_int() == 1);
    REQUIRE(val.as_list()[1].as_int() == 2);
    REQUIRE(val.as_list()[2].as_int() == 3);
    REQUIRE(val.type_name() == "List");
}

TEST_CASE("Value: Tuple construction and type checking", "[compiler][value]") {
    std::vector<Value> elements;
    elements.push_back(Value(int64_t{42}));
    elements.push_back(Value(std::string("hello")));

    Value val(std::move(elements), true);  // true = tuple

    REQUIRE(val.is_tuple());
    REQUIRE(!val.is_list());
    REQUIRE(val.as_tuple().size() == 2);
    REQUIRE(val.as_tuple()[0].as_int() == 42);
    REQUIRE(val.as_tuple()[1].as_string() == "hello");
    REQUIRE(val.type_name() == "Tuple");
}

TEST_CASE("Value: Function construction", "[compiler][value]") {
    Value val = Value::make_function(0, "main");

    REQUIRE(val.is_function());
    REQUIRE(val.as_function_index() == 0);
    REQUIRE(val.as_function_name() == "main");
    REQUIRE(val.type_name() == "Function");
}

TEST_CASE("Value: Copy semantics", "[compiler][value]") {
    Value original(int64_t{42});
    Value copy = original;

    REQUIRE(copy.is_int());
    REQUIRE(copy.as_int() == 42);
    REQUIRE(original.as_int() == 42);  // Original unchanged
}

TEST_CASE("Value: Copy semantics with heap-allocated types", "[compiler][value]") {
    Value original(std::string("hello"));
    Value copy = original;

    REQUIRE(copy.is_string());
    REQUIRE(copy.as_string() == "hello");
    REQUIRE(original.as_string() == "hello");
}

TEST_CASE("Value: Move semantics", "[compiler][value]") {
    Value original(std::string("hello"));
    Value moved = std::move(original);

    REQUIRE(moved.is_string());
    REQUIRE(moved.as_string() == "hello");
}

TEST_CASE("Value: Equality - Int", "[compiler][value]") {
    Value a(int64_t{42});
    Value b(int64_t{42});
    Value c(int64_t{43});

    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("Value: Equality - String", "[compiler][value]") {
    Value a(std::string("hello"));
    Value b(std::string("hello"));
    Value c(std::string("world"));

    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("Value: Comparison - Int", "[compiler][value]") {
    Value a(int64_t{10});
    Value b(int64_t{20});

    REQUIRE(a < b);
    REQUIRE(b > a);
    REQUIRE(a <= b);
    REQUIRE(b >= a);
}

TEST_CASE("Value: Comparison - Float", "[compiler][value]") {
    Value a(1.5);
    Value b(2.5);

    REQUIRE(a < b);
    REQUIRE(b > a);
}

TEST_CASE("Value: Comparison - String", "[compiler][value]") {
    Value a(std::string("apple"));
    Value b(std::string("banana"));

    REQUIRE(a < b);
    REQUIRE(b > a);
}

TEST_CASE("Value: Truthiness", "[compiler][value]") {
    REQUIRE(Value(int64_t{1}).is_truthy());
    REQUIRE(!Value(int64_t{0}).is_truthy());
    REQUIRE(Value(1.5).is_truthy());
    REQUIRE(!Value(0.0).is_truthy());
    REQUIRE(Value(true).is_truthy());
    REQUIRE(!Value(false).is_truthy());
    REQUIRE(Value(std::string("hello")).is_truthy());
    REQUIRE(!Value(std::string("")).is_truthy());
}

TEST_CASE("Value: String representation - primitives", "[compiler][value]") {
    REQUIRE(Value(int64_t{42}).to_string() == "42");
    REQUIRE(Value(3.14).to_string() == "3.14");
    REQUIRE(Value(true).to_string() == "true");
    REQUIRE(Value(false).to_string() == "false");
    REQUIRE(Value(std::string("hello")).to_string() == "\"hello\"");
}

TEST_CASE("Value: String representation - list", "[compiler][value]") {
    std::vector<Value> elements;
    elements.push_back(Value(int64_t{1}));
    elements.push_back(Value(int64_t{2}));
    elements.push_back(Value(int64_t{3}));

    Value list(std::move(elements), false);
    REQUIRE(list.to_string() == "[1, 2, 3]");
}

TEST_CASE("Value: String representation - tuple", "[compiler][value]") {
    std::vector<Value> elements;
    elements.push_back(Value(int64_t{42}));
    elements.push_back(Value(std::string("hello")));

    Value tuple(std::move(elements), true);
    REQUIRE(tuple.to_string() == "(42, \"hello\")");
}

// ===== Bytecode Tests =====

TEST_CASE("Bytecode: Emit simple opcodes", "[compiler][bytecode]") {
    Bytecode bc;

    bc.emit(OpCode::TRUE);
    bc.emit(OpCode::FALSE);
    bc.emit(OpCode::ADD);

    REQUIRE(bc.instructions.size() == 3);
    REQUIRE(bc.instructions[0] == static_cast<uint8_t>(OpCode::TRUE));
    REQUIRE(bc.instructions[1] == static_cast<uint8_t>(OpCode::FALSE));
    REQUIRE(bc.instructions[2] == static_cast<uint8_t>(OpCode::ADD));
}

TEST_CASE("Bytecode: Emit opcode with uint16_t operand", "[compiler][bytecode]") {
    Bytecode bc;

    bc.emit(OpCode::LOAD_LOCAL, uint16_t(42));

    REQUIRE(bc.instructions.size() == 3);
    REQUIRE(bc.instructions[0] == static_cast<uint8_t>(OpCode::LOAD_LOCAL));
    // Little-endian: 42 = 0x002A
    REQUIRE(bc.instructions[1] == 0x2A);
    REQUIRE(bc.instructions[2] == 0x00);
}

TEST_CASE("Bytecode: Emit opcode with uint16_t and uint8_t operands", "[compiler][bytecode]") {
    Bytecode bc;

    bc.emit(OpCode::CALL, uint16_t(5), uint8_t(3));

    REQUIRE(bc.instructions.size() == 4);
    REQUIRE(bc.instructions[0] == static_cast<uint8_t>(OpCode::CALL));
    REQUIRE(bc.instructions[1] == 5);
    REQUIRE(bc.instructions[2] == 0);
    REQUIRE(bc.instructions[3] == 3);
}

TEST_CASE("Bytecode: Add constants", "[compiler][bytecode]") {
    Bytecode bc;

    auto idx1 = bc.add_constant(Value(int64_t{42}));
    auto idx2 = bc.add_constant(Value(3.14));
    auto idx3 = bc.add_constant(Value(std::string("hello")));

    REQUIRE(idx1 == 0);
    REQUIRE(idx2 == 1);
    REQUIRE(idx3 == 2);
    REQUIRE(bc.constants.size() == 3);
    REQUIRE(bc.constants[0].as_int() == 42);
    REQUIRE(bc.constants[1].as_float() == 3.14);
    REQUIRE(bc.constants[2].as_string() == "hello");
}

TEST_CASE("Bytecode: Add and find functions", "[compiler][bytecode]") {
    Bytecode bc;

    auto idx1 = bc.add_function("main", 0, 0, 0);
    auto idx2 = bc.add_function("add", 10, 2, 2);

    REQUIRE(idx1 == 0);
    REQUIRE(idx2 == 1);
    REQUIRE(bc.function_count() == 2);
    REQUIRE(bc.has_function("main"));
    REQUIRE(bc.has_function("add"));
    REQUIRE(!bc.has_function("nonexistent"));

    REQUIRE(bc.find_function("main") == 0);
    REQUIRE(bc.find_function("add") == 1);
    REQUIRE(bc.find_function("nonexistent") == -1);
}

TEST_CASE("Bytecode: Patch jump", "[compiler][bytecode]") {
    Bytecode bc;

    bc.emit(OpCode::JUMP, uint16_t(0));  // Placeholder
    size_t jump_offset = bc.current_offset() - 3;

    bc.emit(OpCode::TRUE);
    bc.emit(OpCode::TRUE);

    // Patch the jump to skip 2 instructions
    bc.patch_jump(jump_offset, 2);

    // Verify patch
    uint16_t patched = static_cast<uint16_t>(
        static_cast<uint16_t>(bc.instructions[jump_offset + 1]) |
        (static_cast<uint16_t>(bc.instructions[jump_offset + 2]) << 8)
    );
    REQUIRE(static_cast<int16_t>(patched) == 2);
}

TEST_CASE("Bytecode: Disassemble simple program", "[compiler][bytecode]") {
    Bytecode bc;

    bc.add_constant(Value(int64_t{42}));
    bc.emit(OpCode::CONSTANT, uint16_t(0));
    bc.emit(OpCode::TRUE);
    bc.emit(OpCode::ADD);
    bc.emit(OpCode::RETURN);

    auto disassembly = bc.disassemble("Test");

    REQUIRE(disassembly.find("== Test ==") != std::string::npos);
    REQUIRE(disassembly.find("CONSTANT") != std::string::npos);
    REQUIRE(disassembly.find("TRUE") != std::string::npos);
    REQUIRE(disassembly.find("ADD") != std::string::npos);
    REQUIRE(disassembly.find("RETURN") != std::string::npos);
}

TEST_CASE("OpCode utilities", "[compiler][bytecode]") {
    REQUIRE(opcode_name(OpCode::ADD) == "ADD");
    REQUIRE(opcode_name(OpCode::CONSTANT) == "CONSTANT");
    REQUIRE(opcode_name(OpCode::RETURN) == "RETURN");

    REQUIRE(opcode_has_operand(OpCode::ADD) == false);
    REQUIRE(opcode_has_operand(OpCode::CONSTANT) == true);
    REQUIRE(opcode_has_operand(OpCode::CALL) == true);

    REQUIRE(opcode_operand_size(OpCode::ADD) == 0);
    REQUIRE(opcode_operand_size(OpCode::CONSTANT) == 2);
    REQUIRE(opcode_operand_size(OpCode::CALL) == 3);
}

// ===== Compiler Tests =====

using namespace lucid::ast;

// Helper to create a simple function for testing
auto make_simple_function(std::string name, std::unique_ptr<BlockExpr> body) -> std::unique_ptr<FunctionDef> {
    std::vector<std::unique_ptr<Parameter>> params;
    auto return_type = std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0});
    return std::make_unique<FunctionDef>(
        std::move(name),
        std::move(params),
        std::move(return_type),
        std::move(body),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );
}

TEST_CASE("Compiler: Compile function with int literal", "[compiler][codegen]") {
    // Create: function test() returns Int { return 42 }
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto literal = std::make_unique<IntLiteralExpr>(int64_t{42}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Compile
    Compiler compiler;
    auto bytecode = compiler.compile(&program);

    // Verify
    REQUIRE(bytecode.function_count() == 1);
    REQUIRE(bytecode.has_function("test"));
    REQUIRE(!bytecode.instructions.empty());
    REQUIRE(bytecode.constants.size() == 1);
    REQUIRE(bytecode.constants[0].as_int() == 42);
}

TEST_CASE("Compiler: Compile function with binary operation", "[compiler][codegen]") {
    // Create: function add() returns Int { return 10 + 20 }
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto left = std::make_unique<IntLiteralExpr>(int64_t{10}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto right = std::make_unique<IntLiteralExpr>(int64_t{20}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto binary = std::make_unique<BinaryExpr>(
        BinaryOp::Add,
        std::move(left),
        std::move(right),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(binary), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("add", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Compile
    Compiler compiler;
    auto bytecode = compiler.compile(&program);

    // Verify bytecode contains ADD instruction
    bool has_add = false;
    for (auto instr : bytecode.instructions) {
        if (instr == static_cast<uint8_t>(OpCode::ADD)) {
            has_add = true;
            break;
        }
    }
    REQUIRE(has_add);
    REQUIRE(bytecode.constants.size() == 2);
}

TEST_CASE("Compiler: Compile function with parameter", "[compiler][codegen]") {
    // Create: function double(x: Int) returns Int { return x + x }
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto left = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto right = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto binary = std::make_unique<BinaryExpr>(
        BinaryOp::Add,
        std::move(left),
        std::move(right),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(binary), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Create function with parameter
    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "x",
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "double",
        std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Compile
    Compiler compiler;
    auto bytecode = compiler.compile(&program);

    // Verify function has 1 parameter and uses LOAD_LOCAL
    REQUIRE(bytecode.functions[0].param_count == 1);
    REQUIRE(bytecode.functions[0].local_count >= 1);

    // Check for LOAD_LOCAL instructions
    bool has_load_local = false;
    for (size_t i = 0; i < bytecode.instructions.size(); ++i) {
        if (bytecode.instructions[i] == static_cast<uint8_t>(OpCode::LOAD_LOCAL)) {
            has_load_local = true;
            break;
        }
    }
    REQUIRE(has_load_local);
}

TEST_CASE("Compiler: Bytecode disassembly", "[compiler][codegen]") {
    // Create simple function
    std::vector<std::unique_ptr<Stmt>> stmts;
    auto literal = std::make_unique<IntLiteralExpr>(int64_t{100}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("main", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Compile
    Compiler compiler;
    auto bytecode = compiler.compile(&program);

    // Disassemble and check it's readable
    auto disassembly = bytecode.disassemble("Test Program");
    REQUIRE(!disassembly.empty());
    REQUIRE(disassembly.find("main") != std::string::npos);
    REQUIRE(disassembly.find("CONSTANT") != std::string::npos);
    REQUIRE(disassembly.find("RETURN") != std::string::npos);
}

// ===== Day 3: Comprehensive Operator Tests =====

// Helper to check if bytecode contains an opcode
bool bytecode_contains(const Bytecode& bc, OpCode opcode) {
    auto target = static_cast<uint8_t>(opcode);
    for (auto instr : bc.instructions) {
        if (instr == target) return true;
    }
    return false;
}

// Helper to create a binary operation test function
auto make_binary_op_test(BinaryOp op, int64_t left_val, int64_t right_val) -> Bytecode {
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto left = std::make_unique<IntLiteralExpr>(left_val, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto right = std::make_unique<IntLiteralExpr>(right_val, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto binary = std::make_unique<BinaryExpr>(
        op, std::move(left), std::move(right), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(binary), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    return compiler.compile(&program);
}

TEST_CASE("Compiler: Arithmetic operators", "[compiler][codegen][day3]") {
    SECTION("Subtraction") {
        auto bc = make_binary_op_test(BinaryOp::Sub, 20, 5);
        REQUIRE(bytecode_contains(bc, OpCode::SUB));
    }

    SECTION("Multiplication") {
        auto bc = make_binary_op_test(BinaryOp::Mul, 6, 7);
        REQUIRE(bytecode_contains(bc, OpCode::MUL));
    }

    SECTION("Division") {
        auto bc = make_binary_op_test(BinaryOp::Div, 20, 4);
        REQUIRE(bytecode_contains(bc, OpCode::DIV));
    }

    SECTION("Modulo") {
        auto bc = make_binary_op_test(BinaryOp::Mod, 10, 3);
        REQUIRE(bytecode_contains(bc, OpCode::MOD));
    }

    SECTION("Power") {
        auto bc = make_binary_op_test(BinaryOp::Pow, 2, 8);
        REQUIRE(bytecode_contains(bc, OpCode::POW));
    }
}

TEST_CASE("Compiler: Comparison operators", "[compiler][codegen][day3]") {
    SECTION("Equal") {
        auto bc = make_binary_op_test(BinaryOp::Eq, 5, 5);
        REQUIRE(bytecode_contains(bc, OpCode::EQ));
    }

    SECTION("Not equal") {
        auto bc = make_binary_op_test(BinaryOp::Ne, 5, 10);
        REQUIRE(bytecode_contains(bc, OpCode::NE));
    }

    SECTION("Less than") {
        auto bc = make_binary_op_test(BinaryOp::Lt, 5, 10);
        REQUIRE(bytecode_contains(bc, OpCode::LT));
    }

    SECTION("Greater than") {
        auto bc = make_binary_op_test(BinaryOp::Gt, 10, 5);
        REQUIRE(bytecode_contains(bc, OpCode::GT));
    }

    SECTION("Less or equal") {
        auto bc = make_binary_op_test(BinaryOp::Le, 5, 10);
        REQUIRE(bytecode_contains(bc, OpCode::LE));
    }

    SECTION("Greater or equal") {
        auto bc = make_binary_op_test(BinaryOp::Ge, 10, 5);
        REQUIRE(bytecode_contains(bc, OpCode::GE));
    }
}

TEST_CASE("Compiler: Logical operators", "[compiler][codegen][day3]") {
    SECTION("Logical AND") {
        auto bc = make_binary_op_test(BinaryOp::And, 1, 1);
        REQUIRE(bytecode_contains(bc, OpCode::AND));
    }

    SECTION("Logical OR") {
        auto bc = make_binary_op_test(BinaryOp::Or, 0, 1);
        REQUIRE(bytecode_contains(bc, OpCode::OR));
    }
}

TEST_CASE("Compiler: Unary operators", "[compiler][codegen][day3]") {
    SECTION("Logical NOT") {
        std::vector<std::unique_ptr<Stmt>> stmts;

        auto operand = std::make_unique<BoolLiteralExpr>(true, lucid::SourceLocation{"", 0, 0, 0, 0});
        auto unary = std::make_unique<UnaryExpr>(
            UnaryOp::Not, std::move(operand), lucid::SourceLocation{"", 0, 0, 0, 0}
        );

        auto return_stmt = std::make_unique<ReturnStmt>(std::move(unary), lucid::SourceLocation{"", 0, 0, 0, 0});
        stmts.push_back(std::move(return_stmt));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("test", std::move(body));

        std::vector<std::unique_ptr<FunctionDef>> functions;
        functions.push_back(std::move(function));

        auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

        Compiler compiler;
        auto bc = compiler.compile(&program);

        REQUIRE(bytecode_contains(bc, OpCode::NOT));
    }

    SECTION("Negation") {
        std::vector<std::unique_ptr<Stmt>> stmts;

        auto operand = std::make_unique<IntLiteralExpr>(int64_t{42}, lucid::SourceLocation{"", 0, 0, 0, 0});
        auto unary = std::make_unique<UnaryExpr>(
            UnaryOp::Neg, std::move(operand), lucid::SourceLocation{"", 0, 0, 0, 0}
        );

        auto return_stmt = std::make_unique<ReturnStmt>(std::move(unary), lucid::SourceLocation{"", 0, 0, 0, 0});
        stmts.push_back(std::move(return_stmt));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("test", std::move(body));

        std::vector<std::unique_ptr<FunctionDef>> functions;
        functions.push_back(std::move(function));

        auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

        Compiler compiler;
        auto bc = compiler.compile(&program);

        REQUIRE(bytecode_contains(bc, OpCode::NEGATE));
    }

    SECTION("Positive (unary +)") {
        std::vector<std::unique_ptr<Stmt>> stmts;

        auto operand = std::make_unique<IntLiteralExpr>(int64_t{42}, lucid::SourceLocation{"", 0, 0, 0, 0});
        auto unary = std::make_unique<UnaryExpr>(
            UnaryOp::Pos, std::move(operand), lucid::SourceLocation{"", 0, 0, 0, 0}
        );

        auto return_stmt = std::make_unique<ReturnStmt>(std::move(unary), lucid::SourceLocation{"", 0, 0, 0, 0});
        stmts.push_back(std::move(return_stmt));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("test", std::move(body));

        std::vector<std::unique_ptr<FunctionDef>> functions;
        functions.push_back(std::move(function));

        auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

        Compiler compiler;
        auto bc = compiler.compile(&program);

        REQUIRE(bytecode_contains(bc, OpCode::POSITIVE));
    }
}

TEST_CASE("Compiler: Float literals", "[compiler][codegen][day3]") {
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto literal = std::make_unique<FloatLiteralExpr>(3.14, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    REQUIRE(bc.constants.size() == 1);
    REQUIRE(bc.constants[0].is_float());
    REQUIRE(bc.constants[0].as_float() == 3.14);
}

TEST_CASE("Compiler: String literals", "[compiler][codegen][day3]") {
    std::vector<std::unique_ptr<Stmt>> stmts;

    auto literal = std::make_unique<StringLiteralExpr>("hello world", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    REQUIRE(bc.constants.size() == 1);
    REQUIRE(bc.constants[0].is_string());
    REQUIRE(bc.constants[0].as_string() == "hello world");
}

TEST_CASE("Compiler: Bool literals", "[compiler][codegen][day3]") {
    SECTION("True literal") {
        std::vector<std::unique_ptr<Stmt>> stmts;

        auto literal = std::make_unique<BoolLiteralExpr>(true, lucid::SourceLocation{"", 0, 0, 0, 0});
        auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
        stmts.push_back(std::move(return_stmt));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("test", std::move(body));

        std::vector<std::unique_ptr<FunctionDef>> functions;
        functions.push_back(std::move(function));

        auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

        Compiler compiler;
        auto bc = compiler.compile(&program);

        REQUIRE(bytecode_contains(bc, OpCode::TRUE));
    }

    SECTION("False literal") {
        std::vector<std::unique_ptr<Stmt>> stmts;

        auto literal = std::make_unique<BoolLiteralExpr>(false, lucid::SourceLocation{"", 0, 0, 0, 0});
        auto return_stmt = std::make_unique<ReturnStmt>(std::move(literal), lucid::SourceLocation{"", 0, 0, 0, 0});
        stmts.push_back(std::move(return_stmt));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("test", std::move(body));

        std::vector<std::unique_ptr<FunctionDef>> functions;
        functions.push_back(std::move(function));

        auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

        Compiler compiler;
        auto bc = compiler.compile(&program);

        REQUIRE(bytecode_contains(bc, OpCode::FALSE));
    }
}

TEST_CASE("Compiler: Complex expression with operator precedence", "[compiler][codegen][day3]") {
    // Test: 2 + 3 * 4 (should be evaluated as 2 + (3 * 4) = 14)
    // Parser handles precedence, compiler just emits in post-order

    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build 3 * 4
    auto mul_left = std::make_unique<IntLiteralExpr>(int64_t{3}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto mul_right = std::make_unique<IntLiteralExpr>(int64_t{4}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto mul = std::make_unique<BinaryExpr>(
        BinaryOp::Mul, std::move(mul_left), std::move(mul_right), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // Build 2 + (3 * 4)
    auto add_left = std::make_unique<IntLiteralExpr>(int64_t{2}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto add = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(add_left), std::move(mul), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(add), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify both operations are present
    REQUIRE(bytecode_contains(bc, OpCode::MUL));
    REQUIRE(bytecode_contains(bc, OpCode::ADD));
    REQUIRE(bc.constants.size() == 3);  // 2, 3, 4
}

// ===== Day 4: Collections & Control Flow Tests =====

TEST_CASE("Compiler: List construction", "[compiler][codegen][day4]") {
    // Test: [1, 2, 3]
    std::vector<std::unique_ptr<Stmt>> stmts;

    std::vector<std::unique_ptr<Expr>> elements;
    elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{1}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{2}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{3}, lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto list = std::make_unique<ListExpr>(std::move(elements), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(list), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify BUILD_LIST instruction is present
    REQUIRE(bytecode_contains(bc, OpCode::BUILD_LIST));
    // Verify all elements are in constant pool
    REQUIRE(bc.constants.size() == 3);
}

TEST_CASE("Compiler: Empty list", "[compiler][codegen][day4]") {
    std::vector<std::unique_ptr<Stmt>> stmts;

    std::vector<std::unique_ptr<Expr>> elements;  // Empty
    auto list = std::make_unique<ListExpr>(std::move(elements), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(list), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    REQUIRE(bytecode_contains(bc, OpCode::BUILD_LIST));
}

TEST_CASE("Compiler: Tuple construction", "[compiler][codegen][day4]") {
    // Test: (42, "hello", true)
    std::vector<std::unique_ptr<Stmt>> stmts;

    std::vector<std::unique_ptr<Expr>> elements;
    elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{42}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    elements.push_back(std::make_unique<StringLiteralExpr>("hello", lucid::SourceLocation{"", 0, 0, 0, 0}));
    elements.push_back(std::make_unique<BoolLiteralExpr>(true, lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto tuple = std::make_unique<TupleExpr>(std::move(elements), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto return_stmt = std::make_unique<ReturnStmt>(std::move(tuple), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify BUILD_TUPLE instruction is present
    REQUIRE(bytecode_contains(bc, OpCode::BUILD_TUPLE));
    // 42 and "hello" should be in constant pool (true uses TRUE opcode)
    REQUIRE(bc.constants.size() == 2);
}

TEST_CASE("Compiler: List indexing", "[compiler][codegen][day4]") {
    // Test: list[0] where list is a parameter
    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build list[0]
    auto list = std::make_unique<IdentifierExpr>("list", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto index = std::make_unique<IntLiteralExpr>(int64_t{0}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto index_expr = std::make_unique<IndexExpr>(
        std::move(list), std::move(index), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(index_expr), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Create function with list parameter
    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "list",
        std::make_unique<ListType>(
            std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
            lucid::SourceLocation{"", 0, 0, 0, 0}
        ),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "get_first",
        std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify INDEX instruction is present
    REQUIRE(bytecode_contains(bc, OpCode::INDEX));
    REQUIRE(bytecode_contains(bc, OpCode::LOAD_LOCAL));
}

TEST_CASE("Compiler: If expression with both branches", "[compiler][codegen][day4]") {
    // Test: if x > 0 { 10 } else { 20 }
    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build condition: x > 0
    auto x = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto zero = std::make_unique<IntLiteralExpr>(int64_t{0}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto condition = std::make_unique<BinaryExpr>(
        BinaryOp::Gt, std::move(x), std::move(zero), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // Build then branch: { 10 }
    std::vector<std::unique_ptr<Stmt>> then_stmts;
    auto then_expr = std::make_unique<IntLiteralExpr>(int64_t{10}, lucid::SourceLocation{"", 0, 0, 0, 0});
    then_stmts.push_back(std::make_unique<ExprStmt>(std::move(then_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto then_branch = std::make_unique<BlockExpr>(std::move(then_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Build else branch: { 20 }
    std::vector<std::unique_ptr<Stmt>> else_stmts;
    auto else_expr = std::make_unique<IntLiteralExpr>(int64_t{20}, lucid::SourceLocation{"", 0, 0, 0, 0});
    else_stmts.push_back(std::make_unique<ExprStmt>(std::move(else_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto else_branch = std::make_unique<BlockExpr>(std::move(else_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Build if expression
    auto if_expr = std::make_unique<IfExpr>(
        std::move(condition),
        std::move(then_branch),
        std::move(else_branch),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(if_expr), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Create function with x parameter
    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "x",
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "test_if",
        std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify conditional jump instructions
    REQUIRE(bytecode_contains(bc, OpCode::JUMP_IF_FALSE));
    REQUIRE(bytecode_contains(bc, OpCode::JUMP));
    REQUIRE(bytecode_contains(bc, OpCode::GT));
}

TEST_CASE("Compiler: If expression without else", "[compiler][codegen][day4]") {
    // Test: if x > 0 { 10 } (no else branch)
    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build condition: x > 0
    auto x = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto zero = std::make_unique<IntLiteralExpr>(int64_t{0}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto condition = std::make_unique<BinaryExpr>(
        BinaryOp::Gt, std::move(x), std::move(zero), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // Build then branch: { 10 }
    std::vector<std::unique_ptr<Stmt>> then_stmts;
    auto then_expr = std::make_unique<IntLiteralExpr>(int64_t{10}, lucid::SourceLocation{"", 0, 0, 0, 0});
    then_stmts.push_back(std::make_unique<ExprStmt>(std::move(then_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto then_branch = std::make_unique<BlockExpr>(std::move(then_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Build if expression (no else)
    auto if_expr = std::make_unique<IfExpr>(
        std::move(condition),
        std::move(then_branch),
        std::nullopt,  // No else branch
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    auto return_stmt = std::make_unique<ReturnStmt>(std::move(if_expr), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::move(return_stmt));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Create function with x parameter
    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "x",
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "test_if",
        std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify jump instructions (compiler adds FALSE for missing else)
    REQUIRE(bytecode_contains(bc, OpCode::JUMP_IF_FALSE));
    REQUIRE(bytecode_contains(bc, OpCode::JUMP));
    REQUIRE(bytecode_contains(bc, OpCode::FALSE));  // Default else value
}

// ===== Day 5: Functions & Statements Tests =====

TEST_CASE("Compiler: Function call with arguments", "[compiler][codegen][day5]") {
    // Create two functions: add(x, y) and main() that calls add(3, 4)
    std::vector<std::unique_ptr<FunctionDef>> functions;

    // Function 1: add(x: Int, y: Int) returns Int { return x + y }
    {
        std::vector<std::unique_ptr<Stmt>> stmts;
        auto x = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
        auto y = std::make_unique<IdentifierExpr>("y", lucid::SourceLocation{"", 0, 0, 0, 0});
        auto add_expr = std::make_unique<BinaryExpr>(
            BinaryOp::Add, std::move(x), std::move(y), lucid::SourceLocation{"", 0, 0, 0, 0}
        );
        stmts.push_back(std::make_unique<ReturnStmt>(std::move(add_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

        std::vector<std::unique_ptr<Parameter>> params;
        params.push_back(std::make_unique<Parameter>(
            "x", std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}), lucid::SourceLocation{"", 0, 0, 0, 0}
        ));
        params.push_back(std::make_unique<Parameter>(
            "y", std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}), lucid::SourceLocation{"", 0, 0, 0, 0}
        ));

        auto function = std::make_unique<FunctionDef>(
            "add", std::move(params),
            std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
            std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
        );
        functions.push_back(std::move(function));
    }

    // Function 2: main() returns Int { return add(3, 4) }
    {
        std::vector<std::unique_ptr<Stmt>> stmts;

        // Build add(3, 4)
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::make_unique<IntLiteralExpr>(int64_t{3}, lucid::SourceLocation{"", 0, 0, 0, 0}));
        args.push_back(std::make_unique<IntLiteralExpr>(int64_t{4}, lucid::SourceLocation{"", 0, 0, 0, 0}));

        auto callee = std::make_unique<IdentifierExpr>("add", lucid::SourceLocation{"", 0, 0, 0, 0});
        auto call = std::make_unique<CallExpr>(
            std::move(callee), std::move(args), lucid::SourceLocation{"", 0, 0, 0, 0}
        );

        stmts.push_back(std::make_unique<ReturnStmt>(std::move(call), lucid::SourceLocation{"", 0, 0, 0, 0}));

        auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
        auto function = make_simple_function("main", std::move(body));
        functions.push_back(std::move(function));
    }

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify function table
    REQUIRE(bc.function_count() == 2);
    REQUIRE(bc.has_function("add"));
    REQUIRE(bc.has_function("main"));

    // Verify CALL instruction
    REQUIRE(bytecode_contains(bc, OpCode::CALL));
}

TEST_CASE("Compiler: Recursive function (fibonacci)", "[compiler][codegen][day5]") {
    // function fibonacci(n: Int) returns Int {
    //   return if n <= 1 { n } else { fibonacci(n-1) + fibonacci(n-2) }
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build n <= 1
    auto n1 = std::make_unique<IdentifierExpr>("n", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto one1 = std::make_unique<IntLiteralExpr>(int64_t{1}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto condition = std::make_unique<BinaryExpr>(
        BinaryOp::Le, std::move(n1), std::move(one1), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // Then branch: { n }
    std::vector<std::unique_ptr<Stmt>> then_stmts;
    auto n2 = std::make_unique<IdentifierExpr>("n", lucid::SourceLocation{"", 0, 0, 0, 0});
    then_stmts.push_back(std::make_unique<ExprStmt>(std::move(n2), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto then_branch = std::make_unique<BlockExpr>(std::move(then_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Else branch: { fibonacci(n-1) + fibonacci(n-2) }
    std::vector<std::unique_ptr<Stmt>> else_stmts;

    // fibonacci(n-1)
    std::vector<std::unique_ptr<Expr>> args1;
    auto n3 = std::make_unique<IdentifierExpr>("n", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto one2 = std::make_unique<IntLiteralExpr>(int64_t{1}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto n_minus_1 = std::make_unique<BinaryExpr>(
        BinaryOp::Sub, std::move(n3), std::move(one2), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    args1.push_back(std::move(n_minus_1));
    auto fib1_callee = std::make_unique<IdentifierExpr>("fibonacci", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto fib1 = std::make_unique<CallExpr>(
        std::move(fib1_callee), std::move(args1), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // fibonacci(n-2)
    std::vector<std::unique_ptr<Expr>> args2;
    auto n4 = std::make_unique<IdentifierExpr>("n", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto two = std::make_unique<IntLiteralExpr>(int64_t{2}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto n_minus_2 = std::make_unique<BinaryExpr>(
        BinaryOp::Sub, std::move(n4), std::move(two), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    args2.push_back(std::move(n_minus_2));
    auto fib2_callee = std::make_unique<IdentifierExpr>("fibonacci", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto fib2 = std::make_unique<CallExpr>(
        std::move(fib2_callee), std::move(args2), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    // fib1 + fib2
    auto add = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(fib1), std::move(fib2), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    else_stmts.push_back(std::make_unique<ExprStmt>(std::move(add), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto else_branch = std::make_unique<BlockExpr>(std::move(else_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    // If expression
    auto if_expr = std::make_unique<IfExpr>(
        std::move(condition), std::move(then_branch), std::move(else_branch),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(if_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "n", std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "fibonacci", std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify recursive call is present
    REQUIRE(bytecode_contains(bc, OpCode::CALL));
    REQUIRE(bytecode_contains(bc, OpCode::ADD));
    REQUIRE(bytecode_contains(bc, OpCode::SUB));
    REQUIRE(bytecode_contains(bc, OpCode::LE));
}

TEST_CASE("Compiler: Let binding with simple variable", "[compiler][codegen][day5]") {
    // function test() returns Int {
    //   let x = 42
    //   return x
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // let x = 42
    auto init = std::make_unique<IntLiteralExpr>(int64_t{42}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto pattern = std::make_unique<IdentifierPattern>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto let_stmt = std::make_unique<LetStmt>(
        std::move(pattern), std::nullopt, std::move(init), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::move(let_stmt));

    // return x
    auto x = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<ReturnStmt>(std::move(x), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify local variable operations
    REQUIRE(bytecode_contains(bc, OpCode::STORE_LOCAL));
    REQUIRE(bytecode_contains(bc, OpCode::LOAD_LOCAL));
    REQUIRE(bc.functions[0].local_count >= 1);
}

TEST_CASE("Compiler: Let binding with tuple destructuring", "[compiler][codegen][day5]") {
    // function test() returns Int {
    //   let (x, y) = (10, 20)
    //   return x + y
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build (10, 20)
    std::vector<std::unique_ptr<Expr>> tuple_elements;
    tuple_elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{10}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    tuple_elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{20}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto init = std::make_unique<TupleExpr>(std::move(tuple_elements), lucid::SourceLocation{"", 0, 0, 0, 0});

    // Build pattern (x, y)
    std::vector<std::unique_ptr<Pattern>> patterns;
    patterns.push_back(std::make_unique<IdentifierPattern>("x", lucid::SourceLocation{"", 0, 0, 0, 0}));
    patterns.push_back(std::make_unique<IdentifierPattern>("y", lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto pattern = std::make_unique<TuplePattern>(std::move(patterns), lucid::SourceLocation{"", 0, 0, 0, 0});

    // let (x, y) = (10, 20)
    auto let_stmt = std::make_unique<LetStmt>(
        std::move(pattern), std::nullopt, std::move(init), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::move(let_stmt));

    // return x + y
    auto x = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto y = std::make_unique<IdentifierExpr>("y", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto add = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(x), std::move(y), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::make_unique<ReturnStmt>(std::move(add), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify tuple destructuring operations
    REQUIRE(bytecode_contains(bc, OpCode::BUILD_TUPLE));
    REQUIRE(bytecode_contains(bc, OpCode::DUP));        // Duplicate tuple for indexing
    REQUIRE(bytecode_contains(bc, OpCode::INDEX));      // Index into tuple
    REQUIRE(bytecode_contains(bc, OpCode::STORE_LOCAL));
    REQUIRE(bc.functions[0].local_count >= 2);  // x and y
}

TEST_CASE("Compiler: Multiple let bindings", "[compiler][codegen][day5]") {
    // function test() returns Int {
    //   let a = 10
    //   let b = 20
    //   let c = a + b
    //   return c
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // let a = 10
    auto init_a = std::make_unique<IntLiteralExpr>(int64_t{10}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto pattern_a = std::make_unique<IdentifierPattern>("a", lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<LetStmt>(
        std::move(pattern_a), std::nullopt, std::move(init_a), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // let b = 20
    auto init_b = std::make_unique<IntLiteralExpr>(int64_t{20}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto pattern_b = std::make_unique<IdentifierPattern>("b", lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<LetStmt>(
        std::move(pattern_b), std::nullopt, std::move(init_b), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // let c = a + b
    auto a = std::make_unique<IdentifierExpr>("a", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto b = std::make_unique<IdentifierExpr>("b", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto add = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(a), std::move(b), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    auto pattern_c = std::make_unique<IdentifierPattern>("c", lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<LetStmt>(
        std::move(pattern_c), std::nullopt, std::move(add), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // return c
    auto c = std::make_unique<IdentifierExpr>("c", lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<ReturnStmt>(std::move(c), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("test", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify multiple local variables
    REQUIRE(bc.functions[0].local_count >= 3);  // a, b, c
    REQUIRE(bytecode_contains(bc, OpCode::STORE_LOCAL));
    REQUIRE(bytecode_contains(bc, OpCode::LOAD_LOCAL));
    REQUIRE(bytecode_contains(bc, OpCode::ADD));
}

// ===== Day 6: Built-in Methods & Integration Tests =====

TEST_CASE("Compiler: Built-in method - list.length()", "[compiler][codegen][day6]") {
    // function get_length(nums: List[Int]) returns Int {
    //   return nums.length()
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build nums.length()
    auto object = std::make_unique<IdentifierExpr>("nums", lucid::SourceLocation{"", 0, 0, 0, 0});
    std::vector<std::unique_ptr<Expr>> args;  // length() takes no arguments
    auto method_call = std::make_unique<MethodCallExpr>(
        std::move(object), "length", std::move(args), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(method_call), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "nums",
        std::make_unique<ListType>(
            std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
            lucid::SourceLocation{"", 0, 0, 0, 0}
        ),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "get_length", std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify CALL_METHOD instruction
    REQUIRE(bytecode_contains(bc, OpCode::CALL_METHOD));
    // "length" should be in constant pool
    bool has_length = false;
    for (const auto& constant : bc.constants) {
        if (constant.is_string() && constant.as_string() == "length") {
            has_length = true;
            break;
        }
    }
    REQUIRE(has_length);
}

TEST_CASE("Compiler: Built-in method - list.append()", "[compiler][codegen][day6]") {
    // function append_value(nums: List[Int], val: Int) returns List[Int] {
    //   return nums.append(val)
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // Build nums.append(val)
    auto object = std::make_unique<IdentifierExpr>("nums", lucid::SourceLocation{"", 0, 0, 0, 0});
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<IdentifierExpr>("val", lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto method_call = std::make_unique<MethodCallExpr>(
        std::move(object), "append", std::move(args), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(method_call), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "nums",
        std::make_unique<ListType>(
            std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
            lucid::SourceLocation{"", 0, 0, 0, 0}
        ),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));
    params.push_back(std::make_unique<Parameter>(
        "val",
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "append_value", std::move(params),
        std::make_unique<ListType>(
            std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
            lucid::SourceLocation{"", 0, 0, 0, 0}
        ),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify CALL_METHOD instruction with argument
    REQUIRE(bytecode_contains(bc, OpCode::CALL_METHOD));
    REQUIRE(bytecode_contains(bc, OpCode::LOAD_LOCAL));  // Load 'val' argument
}

TEST_CASE("Compiler: Built-in method - string.length()", "[compiler][codegen][day6]") {
    // function string_len(s: String) returns Int {
    //   return s.length()
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    auto object = std::make_unique<IdentifierExpr>("s", lucid::SourceLocation{"", 0, 0, 0, 0});
    std::vector<std::unique_ptr<Expr>> args;
    auto method_call = std::make_unique<MethodCallExpr>(
        std::move(object), "length", std::move(args), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(method_call), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "s",
        std::make_unique<NamedType>("String", lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "string_len", std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    REQUIRE(bytecode_contains(bc, OpCode::CALL_METHOD));
}

TEST_CASE("Compiler: Built-in method - tuple.length()", "[compiler][codegen][day6]") {
    // function tuple_len(t: (Int, String)) returns Int {
    //   return t.length()
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    auto object = std::make_unique<IdentifierExpr>("t", lucid::SourceLocation{"", 0, 0, 0, 0});
    std::vector<std::unique_ptr<Expr>> args;
    auto method_call = std::make_unique<MethodCallExpr>(
        std::move(object), "length", std::move(args), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(method_call), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    std::vector<std::unique_ptr<Type>> tuple_types;
    tuple_types.push_back(std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}));
    tuple_types.push_back(std::make_unique<NamedType>("String", lucid::SourceLocation{"", 0, 0, 0, 0}));
    params.push_back(std::make_unique<Parameter>(
        "t",
        std::make_unique<TupleType>(std::move(tuple_types), lucid::SourceLocation{"", 0, 0, 0, 0}),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "tuple_len", std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    REQUIRE(bytecode_contains(bc, OpCode::CALL_METHOD));
}

TEST_CASE("Compiler: Integration - Complex expression", "[compiler][codegen][day6][integration]") {
    // function complex(x: Int, y: Int) returns Int {
    //   let sum = x + y
    //   let product = x * y
    //   return if sum > product { sum } else { product }
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // let sum = x + y
    auto x1 = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto y1 = std::make_unique<IdentifierExpr>("y", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto sum_init = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(x1), std::move(y1), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::make_unique<LetStmt>(
        std::make_unique<IdentifierPattern>("sum", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::nullopt, std::move(sum_init), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // let product = x * y
    auto x2 = std::make_unique<IdentifierExpr>("x", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto y2 = std::make_unique<IdentifierExpr>("y", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto product_init = std::make_unique<BinaryExpr>(
        BinaryOp::Mul, std::move(x2), std::move(y2), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::make_unique<LetStmt>(
        std::make_unique<IdentifierPattern>("product", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::nullopt, std::move(product_init), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // Build if expression
    auto sum1 = std::make_unique<IdentifierExpr>("sum", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto product1 = std::make_unique<IdentifierExpr>("product", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto condition = std::make_unique<BinaryExpr>(
        BinaryOp::Gt, std::move(sum1), std::move(product1), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<Stmt>> then_stmts;
    auto sum2 = std::make_unique<IdentifierExpr>("sum", lucid::SourceLocation{"", 0, 0, 0, 0});
    then_stmts.push_back(std::make_unique<ExprStmt>(std::move(sum2), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto then_branch = std::make_unique<BlockExpr>(std::move(then_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Stmt>> else_stmts;
    auto product2 = std::make_unique<IdentifierExpr>("product", lucid::SourceLocation{"", 0, 0, 0, 0});
    else_stmts.push_back(std::make_unique<ExprStmt>(std::move(product2), lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto else_branch = std::make_unique<BlockExpr>(std::move(else_stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    auto if_expr = std::make_unique<IfExpr>(
        std::move(condition), std::move(then_branch), std::move(else_branch),
        lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    stmts.push_back(std::make_unique<ReturnStmt>(std::move(if_expr), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});

    std::vector<std::unique_ptr<Parameter>> params;
    params.push_back(std::make_unique<Parameter>(
        "x", std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));
    params.push_back(std::make_unique<Parameter>(
        "y", std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    auto function = std::make_unique<FunctionDef>(
        "complex", std::move(params),
        std::make_unique<NamedType>("Int", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::move(body), lucid::SourceLocation{"", 0, 0, 0, 0}
    );

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify all components are present
    REQUIRE(bytecode_contains(bc, OpCode::ADD));
    REQUIRE(bytecode_contains(bc, OpCode::MUL));
    REQUIRE(bytecode_contains(bc, OpCode::GT));
    REQUIRE(bytecode_contains(bc, OpCode::JUMP_IF_FALSE));
    REQUIRE(bytecode_contains(bc, OpCode::STORE_LOCAL));
    REQUIRE(bytecode_contains(bc, OpCode::LOAD_LOCAL));
    REQUIRE(bc.functions[0].local_count >= 4);  // x, y, sum, product
}

TEST_CASE("Compiler: Integration - List operations", "[compiler][codegen][day6][integration]") {
    // function process_list() returns Int {
    //   let nums = [1, 2, 3]
    //   let first = nums[0]
    //   return first + nums.length()
    // }

    std::vector<std::unique_ptr<Stmt>> stmts;

    // let nums = [1, 2, 3]
    std::vector<std::unique_ptr<Expr>> list_elements;
    list_elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{1}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    list_elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{2}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    list_elements.push_back(std::make_unique<IntLiteralExpr>(int64_t{3}, lucid::SourceLocation{"", 0, 0, 0, 0}));
    auto list = std::make_unique<ListExpr>(std::move(list_elements), lucid::SourceLocation{"", 0, 0, 0, 0});
    stmts.push_back(std::make_unique<LetStmt>(
        std::make_unique<IdentifierPattern>("nums", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::nullopt, std::move(list), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // let first = nums[0]
    auto nums1 = std::make_unique<IdentifierExpr>("nums", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto index = std::make_unique<IntLiteralExpr>(int64_t{0}, lucid::SourceLocation{"", 0, 0, 0, 0});
    auto index_expr = std::make_unique<IndexExpr>(
        std::move(nums1), std::move(index), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::make_unique<LetStmt>(
        std::make_unique<IdentifierPattern>("first", lucid::SourceLocation{"", 0, 0, 0, 0}),
        std::nullopt, std::move(index_expr), lucid::SourceLocation{"", 0, 0, 0, 0}
    ));

    // return first + nums.length()
    auto first = std::make_unique<IdentifierExpr>("first", lucid::SourceLocation{"", 0, 0, 0, 0});
    auto nums2 = std::make_unique<IdentifierExpr>("nums", lucid::SourceLocation{"", 0, 0, 0, 0});
    std::vector<std::unique_ptr<Expr>> method_args;
    auto length_call = std::make_unique<MethodCallExpr>(
        std::move(nums2), "length", std::move(method_args), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    auto add = std::make_unique<BinaryExpr>(
        BinaryOp::Add, std::move(first), std::move(length_call), lucid::SourceLocation{"", 0, 0, 0, 0}
    );
    stmts.push_back(std::make_unique<ReturnStmt>(std::move(add), lucid::SourceLocation{"", 0, 0, 0, 0}));

    auto body = std::make_unique<BlockExpr>(std::move(stmts), lucid::SourceLocation{"", 0, 0, 0, 0});
    auto function = make_simple_function("process_list", std::move(body));

    std::vector<std::unique_ptr<FunctionDef>> functions;
    functions.push_back(std::move(function));

    auto program = Program(std::move(functions), lucid::SourceLocation{"", 0, 0, 0, 0});

    Compiler compiler;
    auto bc = compiler.compile(&program);

    // Verify all list operations
    REQUIRE(bytecode_contains(bc, OpCode::BUILD_LIST));
    REQUIRE(bytecode_contains(bc, OpCode::INDEX));
    REQUIRE(bytecode_contains(bc, OpCode::CALL_METHOD));
    REQUIRE(bytecode_contains(bc, OpCode::ADD));
}
