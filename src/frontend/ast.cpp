#include <lucid/frontend/ast.hpp>

namespace lucid::ast {

// ===== Visitor accept() implementations =====

// Expression visitors
auto IntLiteralExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_int_literal(this);
}

auto FloatLiteralExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_float_literal(this);
}

auto StringLiteralExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_string_literal(this);
}

auto BoolLiteralExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_bool_literal(this);
}

auto IdentifierExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_identifier(this);
}

auto TupleExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_tuple(this);
}

auto ListExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_list(this);
}

auto BinaryExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_binary(this);
}

auto UnaryExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_unary(this);
}

auto CallExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_call(this);
}

auto MethodCallExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_method_call(this);
}

auto IndexExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_index(this);
}

auto LambdaExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_lambda(this);
}

auto IfExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_if(this);
}

auto BlockExpr::accept(ExprVisitor& visitor) -> void {
    visitor.visit_block(this);
}

// Statement visitors
auto LetStmt::accept(StmtVisitor& visitor) -> void {
    visitor.visit_let(this);
}

auto ReturnStmt::accept(StmtVisitor& visitor) -> void {
    visitor.visit_return(this);
}

auto ExprStmt::accept(StmtVisitor& visitor) -> void {
    visitor.visit_expr_stmt(this);
}

// Pattern visitors
auto IdentifierPattern::accept(PatternVisitor& visitor) -> void {
    visitor.visit_identifier_pattern(this);
}

auto TuplePattern::accept(PatternVisitor& visitor) -> void {
    visitor.visit_tuple_pattern(this);
}

// Type visitors
auto NamedType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_named_type(this);
}

auto ListType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_list_type(this);
}

auto TupleType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_tuple_type(this);
}

// ===== Utility Functions =====

auto binary_op_from_token(TokenType type) -> std::optional<BinaryOp> {
    switch (type) {
        case TokenType::Plus: return BinaryOp::Add;
        case TokenType::Minus: return BinaryOp::Sub;
        case TokenType::Star: return BinaryOp::Mul;
        case TokenType::Slash: return BinaryOp::Div;
        case TokenType::Percent: return BinaryOp::Mod;
        case TokenType::Power: return BinaryOp::Pow;
        case TokenType::Equal: return BinaryOp::Eq;
        case TokenType::NotEqual: return BinaryOp::Ne;
        case TokenType::Less: return BinaryOp::Lt;
        case TokenType::Greater: return BinaryOp::Gt;
        case TokenType::LessEqual: return BinaryOp::Le;
        case TokenType::GreaterEqual: return BinaryOp::Ge;
        case TokenType::And: return BinaryOp::And;
        case TokenType::Or: return BinaryOp::Or;
        default: return std::nullopt;
    }
}

auto unary_op_from_token(TokenType type) -> std::optional<UnaryOp> {
    switch (type) {
        case TokenType::Not: return UnaryOp::Not;
        case TokenType::Minus: return UnaryOp::Neg;
        case TokenType::Plus: return UnaryOp::Pos;
        default: return std::nullopt;
    }
}

auto binary_op_name(BinaryOp op) -> std::string_view {
    switch (op) {
        case BinaryOp::Add: return "+";
        case BinaryOp::Sub: return "-";
        case BinaryOp::Mul: return "*";
        case BinaryOp::Div: return "/";
        case BinaryOp::Mod: return "%";
        case BinaryOp::Pow: return "**";
        case BinaryOp::Eq: return "==";
        case BinaryOp::Ne: return "!=";
        case BinaryOp::Lt: return "<";
        case BinaryOp::Gt: return ">";
        case BinaryOp::Le: return "<=";
        case BinaryOp::Ge: return ">=";
        case BinaryOp::And: return "and";
        case BinaryOp::Or: return "or";
        default: return "unknown";
    }
}

auto unary_op_name(UnaryOp op) -> std::string_view {
    switch (op) {
        case UnaryOp::Not: return "not";
        case UnaryOp::Neg: return "-";
        case UnaryOp::Pos: return "+";
        default: return "unknown";
    }
}

} // namespace lucid::ast
