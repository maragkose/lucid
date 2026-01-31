#pragma once

#include <lucid/semantic/type_system.hpp>
#include <lucid/semantic/symbol_table.hpp>
#include <lucid/frontend/ast.hpp>
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace lucid {
namespace semantic {

// ===== Type Error =====

struct TypeError {
    SourceLocation location;
    std::string message;

    TypeError(SourceLocation loc, std::string msg)
        : location(loc), message(std::move(msg)) {}
};

// ===== Type Checking Result =====

struct TypeCheckResult {
    std::vector<TypeError> errors;
    bool success;

    TypeCheckResult() : success(true) {}

    auto add_error(SourceLocation location, std::string message) -> void {
        errors.emplace_back(location, std::move(message));
        success = false;
    }

    auto has_errors() const -> bool { return !errors.empty(); }
};

// ===== Type Checker =====

class TypeChecker : public ast::ExprVisitor, public ast::StmtVisitor {
public:
    TypeChecker();

    // Main entry point - type check a program
    auto check_program(ast::Program& program) -> TypeCheckResult;

    // Type check individual nodes
    auto check_expression(ast::Expr& expr) -> std::unique_ptr<SemanticType>;
    auto check_statement(ast::Stmt& stmt) -> void;
    auto check_function(ast::FunctionDef& func) -> void;

    // ExprVisitor interface
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

    // StmtVisitor interface
    auto visit_let(ast::LetStmt* stmt) -> void override;
    auto visit_return(ast::ReturnStmt* stmt) -> void override;
    auto visit_expr_stmt(ast::ExprStmt* stmt) -> void override;

    // Get errors collected during type checking
    auto get_errors() const -> const std::vector<TypeError>& { return result_.errors; }

private:
    SymbolTable symbol_table_;
    TypeEnvironment type_env_;
    TypeCheckResult result_;

    // Current expression type (set by visit methods)
    std::unique_ptr<SemanticType> current_type_;

    // Current function return type (for checking return statements)
    SemanticType* current_function_return_type_;

    // Helper methods
    auto error(SourceLocation location, std::string message) -> void;
    auto type_mismatch_error(SourceLocation location,
                             const SemanticType& expected,
                             const SemanticType& actual) -> void;

    // Convert AST type to semantic type
    auto ast_type_to_semantic(const ast::Type& ast_type) -> std::unique_ptr<SemanticType>;

    // Type checking helpers
    auto check_binary_arithmetic(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType>;
    auto check_binary_comparison(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType>;
    auto check_binary_logical(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType>;
    auto check_unary_arithmetic(ast::UnaryExpr* expr) -> std::unique_ptr<SemanticType>;
    auto check_unary_logical(ast::UnaryExpr* expr) -> std::unique_ptr<SemanticType>;

    // Pattern checking
    auto check_pattern(ast::Pattern& pattern, const SemanticType& expected_type) -> void;
};

} // namespace semantic
} // namespace lucid
