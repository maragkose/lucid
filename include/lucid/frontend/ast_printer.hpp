#pragma once

#include <lucid/frontend/ast.hpp>
#include <sstream>
#include <string>

namespace lucid {

// AST Printer - Pretty prints AST in tree format for debugging
class ASTPrinter : public ast::ExprVisitor,
                   public ast::StmtVisitor,
                   public ast::PatternVisitor,
                   public ast::TypeVisitor {
public:
    ASTPrinter() = default;

    // Main entry points
    auto print_program(const ast::Program& program) -> std::string;
    auto print_expr(ast::Expr* expr) -> std::string;
    auto print_stmt(ast::Stmt* stmt) -> std::string;
    auto print_pattern(ast::Pattern* pattern) -> std::string;
    auto print_type(ast::Type* type) -> std::string;

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

    // PatternVisitor interface
    auto visit_identifier_pattern(ast::IdentifierPattern* pattern) -> void override;
    auto visit_tuple_pattern(ast::TuplePattern* pattern) -> void override;

    // TypeVisitor interface
    auto visit_named_type(ast::NamedType* type) -> void override;
    auto visit_list_type(ast::ListType* type) -> void override;
    auto visit_tuple_type(ast::TupleType* type) -> void override;

private:
    std::ostringstream output_;
    int indent_level_ = 0;
    static constexpr int INDENT_SIZE = 2;

    // Helper methods
    auto indent() -> void;
    auto write(std::string_view text) -> void;
    auto writeln(std::string_view text) -> void;
    auto with_indent(auto&& func) -> void;
};

} // namespace lucid
