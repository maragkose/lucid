#pragma once

#include <lucid/frontend/ast.hpp>
#include <lucid/backend/bytecode.hpp>
#include <lucid/backend/value.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace lucid::backend {

// Compiler for generating bytecode from typed AST
class Compiler : public ast::ExprVisitor, public ast::StmtVisitor {
public:
    Compiler();
    ~Compiler() override = default;

    // Main compilation entry point
    auto compile(ast::Program* program) -> Bytecode;

    // Expression visitors
    auto visit_int_literal(ast::IntLiteralExpr* expr) -> void override;
    auto visit_float_literal(ast::FloatLiteralExpr* expr) -> void override;
    auto visit_string_literal(ast::StringLiteralExpr* expr) -> void override;
    auto visit_bool_literal(ast::BoolLiteralExpr* expr) -> void override;
    auto visit_identifier(ast::IdentifierExpr* expr) -> void override;
    auto visit_tuple(ast::TupleExpr* expr) -> void override;
    auto visit_list(ast::ListExpr* expr) -> void override;
    auto visit_binary(ast::BinaryExpr* expr) -> void override;
    auto visit_unary(ast::UnaryExpr* expr) -> void override;
    auto visit_call(ast::CallExpr* expr) -> void override;
    auto visit_method_call(ast::MethodCallExpr* expr) -> void override;
    auto visit_index(ast::IndexExpr* expr) -> void override;
    auto visit_lambda(ast::LambdaExpr* expr) -> void override;
    auto visit_if(ast::IfExpr* expr) -> void override;
    auto visit_block(ast::BlockExpr* expr) -> void override;

    // Statement visitors
    auto visit_let(ast::LetStmt* stmt) -> void override;
    auto visit_return(ast::ReturnStmt* stmt) -> void override;
    auto visit_expr_stmt(ast::ExprStmt* stmt) -> void override;

private:
    // Bytecode being built
    Bytecode bytecode_;

    // Local variable scope tracking
    struct Scope {
        std::unordered_map<std::string, size_t> locals;  // name -> index
        size_t local_count = 0;
    };
    std::vector<Scope> scopes_;

    // Current function being compiled
    struct FunctionContext {
        std::string name;
        size_t param_count;
        size_t offset;  // Bytecode offset where function starts
    };
    FunctionContext* current_function_ = nullptr;

    // Function table (for Pass 1)
    std::unordered_map<std::string, size_t> function_indices_;  // name -> index in bytecode.functions

    // Scope management
    auto enter_scope() -> void;
    auto exit_scope() -> void;
    auto declare_local(const std::string& name) -> size_t;  // Returns local index
    auto resolve_local(const std::string& name) -> int;     // Returns -1 if not found
    auto resolve_function(const std::string& name) -> int;  // Returns -1 if not found

    // Bytecode emission helpers
    auto emit(OpCode opcode) -> void;
    auto emit(OpCode opcode, uint8_t operand) -> void;
    auto emit(OpCode opcode, uint16_t operand) -> void;
    auto emit(OpCode opcode, uint16_t operand1, uint8_t operand2) -> void;

    // Constant pool helpers
    auto add_constant(Value value) -> uint16_t;

    // Jump patching helpers
    auto emit_jump(OpCode opcode) -> size_t;  // Returns offset of jump instruction
    auto patch_jump(size_t offset) -> void;

    // Two-pass compilation
    auto collect_functions(ast::Program* program) -> void;  // Pass 1
    auto compile_function(ast::FunctionDef* function) -> void;  // Pass 2

    // Pattern compilation (for let statements)
    auto compile_pattern(ast::Pattern* pattern, bool is_declaration) -> void;
};

} // namespace lucid::backend
