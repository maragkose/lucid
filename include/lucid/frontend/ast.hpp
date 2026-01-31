#pragma once

#include <lucid/frontend/token.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace lucid::ast {

// Forward declarations
class Expr;
class Stmt;
class Type;
class Pattern;
class Parameter;
class FunctionDef;
class Program;

// Visitor forward declarations
class ExprVisitor;
class StmtVisitor;
class TypeVisitor;
class PatternVisitor;

// ===== Expression Nodes =====

enum class ExprKind {
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    BoolLiteral,
    Identifier,
    Tuple,
    List,
    Binary,
    Unary,
    Call,
    MethodCall,
    Index,
    Lambda,
    If,
    Block,
};

enum class BinaryOp {
    // Arithmetic
    Add, Sub, Mul, Div, Mod, Pow,
    // Comparison
    Eq, Ne, Lt, Gt, Le, Ge,
    // Logical
    And, Or,
};

enum class UnaryOp {
    Not, Neg, Pos
};

// Base class for all expressions
class Expr {
public:
    ExprKind kind;
    SourceLocation location;

    explicit Expr(ExprKind kind, SourceLocation location)
        : kind(kind), location(location) {}

    virtual ~Expr() = default;

    // Disable copying, enable moving
    Expr(const Expr&) = delete;
    Expr& operator=(const Expr&) = delete;
    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;

    // Visitor pattern
    virtual auto accept(ExprVisitor& visitor) -> void = 0;
};

// Integer literal
class IntLiteralExpr : public Expr {
public:
    int64_t value;

    IntLiteralExpr(int64_t value, SourceLocation location)
        : Expr(ExprKind::IntLiteral, location), value(value) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Float literal
class FloatLiteralExpr : public Expr {
public:
    double value;

    FloatLiteralExpr(double value, SourceLocation location)
        : Expr(ExprKind::FloatLiteral, location), value(value) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// String literal
class StringLiteralExpr : public Expr {
public:
    std::string value;

    StringLiteralExpr(std::string value, SourceLocation location)
        : Expr(ExprKind::StringLiteral, location), value(std::move(value)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Boolean literal
class BoolLiteralExpr : public Expr {
public:
    bool value;

    BoolLiteralExpr(bool value, SourceLocation location)
        : Expr(ExprKind::BoolLiteral, location), value(value) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Identifier reference
class IdentifierExpr : public Expr {
public:
    std::string name;

    IdentifierExpr(std::string name, SourceLocation location)
        : Expr(ExprKind::Identifier, location), name(std::move(name)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Tuple expression
class TupleExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    TupleExpr(std::vector<std::unique_ptr<Expr>> elements, SourceLocation location)
        : Expr(ExprKind::Tuple, location), elements(std::move(elements)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// List expression
class ListExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    ListExpr(std::vector<std::unique_ptr<Expr>> elements, SourceLocation location)
        : Expr(ExprKind::List, location), elements(std::move(elements)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Binary operation
class BinaryExpr : public Expr {
public:
    BinaryOp op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> left,
               std::unique_ptr<Expr> right, SourceLocation location)
        : Expr(ExprKind::Binary, location), op(op),
          left(std::move(left)), right(std::move(right)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Unary operation
class UnaryExpr : public Expr {
public:
    UnaryOp op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand, SourceLocation location)
        : Expr(ExprKind::Unary, location), op(op), operand(std::move(operand)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Function/lambda call
class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> callee,
             std::vector<std::unique_ptr<Expr>> arguments,
             SourceLocation location)
        : Expr(ExprKind::Call, location), callee(std::move(callee)),
          arguments(std::move(arguments)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Method call
class MethodCallExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string method_name;
    std::vector<std::unique_ptr<Expr>> arguments;

    MethodCallExpr(std::unique_ptr<Expr> object, std::string method_name,
                   std::vector<std::unique_ptr<Expr>> arguments,
                   SourceLocation location)
        : Expr(ExprKind::MethodCall, location), object(std::move(object)),
          method_name(std::move(method_name)), arguments(std::move(arguments)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Index access
class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index,
              SourceLocation location)
        : Expr(ExprKind::Index, location), object(std::move(object)),
          index(std::move(index)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Lambda expression (untyped parameters)
class LambdaExpr : public Expr {
public:
    std::vector<std::string> parameters;  // Untyped parameter names
    std::unique_ptr<Expr> body;  // Expression or BlockExpr

    LambdaExpr(std::vector<std::string> parameters,
               std::unique_ptr<Expr> body, SourceLocation location)
        : Expr(ExprKind::Lambda, location), parameters(std::move(parameters)),
          body(std::move(body)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// If expression
class IfExpr : public Expr {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> then_branch;  // Usually BlockExpr
    std::optional<std::unique_ptr<Expr>> else_branch;  // Usually BlockExpr

    IfExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> then_branch,
           std::optional<std::unique_ptr<Expr>> else_branch, SourceLocation location)
        : Expr(ExprKind::If, location), condition(std::move(condition)),
          then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// Block expression (sequence of statements)
class BlockExpr : public Expr {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockExpr(std::vector<std::unique_ptr<Stmt>> statements, SourceLocation location)
        : Expr(ExprKind::Block, location), statements(std::move(statements)) {}

    auto accept(ExprVisitor& visitor) -> void override;
};

// ===== Statement Nodes =====

enum class StmtKind {
    Let,
    Return,
    ExprStmt,
};

class Stmt {
public:
    StmtKind kind;
    SourceLocation location;

    explicit Stmt(StmtKind kind, SourceLocation location)
        : kind(kind), location(location) {}

    virtual ~Stmt() = default;

    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;
    Stmt(Stmt&&) = default;
    Stmt& operator=(Stmt&&) = default;

    // Visitor pattern
    virtual auto accept(StmtVisitor& visitor) -> void = 0;
};

// Let binding with pattern
class LetStmt : public Stmt {
public:
    std::unique_ptr<Pattern> pattern;
    std::optional<std::unique_ptr<Type>> type_annotation;
    std::unique_ptr<Expr> initializer;

    LetStmt(std::unique_ptr<Pattern> pattern,
            std::optional<std::unique_ptr<Type>> type_annotation,
            std::unique_ptr<Expr> initializer, SourceLocation location)
        : Stmt(StmtKind::Let, location), pattern(std::move(pattern)),
          type_annotation(std::move(type_annotation)),
          initializer(std::move(initializer)) {}

    auto accept(StmtVisitor& visitor) -> void override;
};

// Return statement
class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;

    ReturnStmt(std::unique_ptr<Expr> value, SourceLocation location)
        : Stmt(StmtKind::Return, location), value(std::move(value)) {}

    auto accept(StmtVisitor& visitor) -> void override;
};

// Expression statement
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    ExprStmt(std::unique_ptr<Expr> expression, SourceLocation location)
        : Stmt(StmtKind::ExprStmt, location), expression(std::move(expression)) {}

    auto accept(StmtVisitor& visitor) -> void override;
};

// ===== Pattern Nodes =====

enum class PatternKind {
    Identifier,
    Tuple,
};

class Pattern {
public:
    PatternKind kind;
    SourceLocation location;

    explicit Pattern(PatternKind kind, SourceLocation location)
        : kind(kind), location(location) {}

    virtual ~Pattern() = default;

    Pattern(const Pattern&) = delete;
    Pattern& operator=(const Pattern&) = delete;
    Pattern(Pattern&&) = default;
    Pattern& operator=(Pattern&&) = default;

    // Visitor pattern
    virtual auto accept(PatternVisitor& visitor) -> void = 0;
};

// Identifier pattern
class IdentifierPattern : public Pattern {
public:
    std::string name;

    IdentifierPattern(std::string name, SourceLocation location)
        : Pattern(PatternKind::Identifier, location), name(std::move(name)) {}

    auto accept(PatternVisitor& visitor) -> void override;
};

// Tuple destructuring pattern
class TuplePattern : public Pattern {
public:
    std::vector<std::unique_ptr<Pattern>> elements;

    TuplePattern(std::vector<std::unique_ptr<Pattern>> elements, SourceLocation location)
        : Pattern(PatternKind::Tuple, location), elements(std::move(elements)) {}

    auto accept(PatternVisitor& visitor) -> void override;
};

// ===== Type Nodes =====

enum class TypeKind {
    Named,
    List,
    Tuple,
};

class Type {
public:
    TypeKind kind;
    SourceLocation location;

    explicit Type(TypeKind kind, SourceLocation location)
        : kind(kind), location(location) {}

    virtual ~Type() = default;

    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;

    // Visitor pattern
    virtual auto accept(TypeVisitor& visitor) -> void = 0;
};

// Named type
class NamedType : public Type {
public:
    std::string name;

    NamedType(std::string name, SourceLocation location)
        : Type(TypeKind::Named, location), name(std::move(name)) {}

    auto accept(TypeVisitor& visitor) -> void override;
};

// List type
class ListType : public Type {
public:
    std::unique_ptr<Type> element_type;

    ListType(std::unique_ptr<Type> element_type, SourceLocation location)
        : Type(TypeKind::List, location), element_type(std::move(element_type)) {}

    auto accept(TypeVisitor& visitor) -> void override;
};

// Tuple type
class TupleType : public Type {
public:
    std::vector<std::unique_ptr<Type>> element_types;

    TupleType(std::vector<std::unique_ptr<Type>> element_types, SourceLocation location)
        : Type(TypeKind::Tuple, location), element_types(std::move(element_types)) {}

    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Function Definition =====

// Function parameter
class Parameter {
public:
    std::string name;
    std::unique_ptr<Type> type;
    SourceLocation location;

    Parameter(std::string name, std::unique_ptr<Type> type, SourceLocation location)
        : name(std::move(name)), type(std::move(type)), location(location) {}
};

// Function definition
class FunctionDef {
public:
    std::string name;
    std::vector<std::unique_ptr<Parameter>> parameters;
    std::unique_ptr<Type> return_type;
    std::unique_ptr<BlockExpr> body;
    SourceLocation location;

    FunctionDef(std::string name,
                std::vector<std::unique_ptr<Parameter>> parameters,
                std::unique_ptr<Type> return_type,
                std::unique_ptr<BlockExpr> body,
                SourceLocation location)
        : name(std::move(name)), parameters(std::move(parameters)),
          return_type(std::move(return_type)), body(std::move(body)),
          location(location) {}
};

// ===== Program (Top-level) =====

class Program {
public:
    std::vector<std::unique_ptr<FunctionDef>> functions;
    SourceLocation location;

    Program(std::vector<std::unique_ptr<FunctionDef>> functions,
            SourceLocation location)
        : functions(std::move(functions)), location(location) {}
};

// ===== Visitor Pattern Interfaces =====

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;

    virtual auto visit_int_literal(IntLiteralExpr* expr) -> void = 0;
    virtual auto visit_float_literal(FloatLiteralExpr* expr) -> void = 0;
    virtual auto visit_string_literal(StringLiteralExpr* expr) -> void = 0;
    virtual auto visit_bool_literal(BoolLiteralExpr* expr) -> void = 0;
    virtual auto visit_identifier(IdentifierExpr* expr) -> void = 0;
    virtual auto visit_tuple(TupleExpr* expr) -> void = 0;
    virtual auto visit_list(ListExpr* expr) -> void = 0;
    virtual auto visit_binary(BinaryExpr* expr) -> void = 0;
    virtual auto visit_unary(UnaryExpr* expr) -> void = 0;
    virtual auto visit_call(CallExpr* expr) -> void = 0;
    virtual auto visit_method_call(MethodCallExpr* expr) -> void = 0;
    virtual auto visit_index(IndexExpr* expr) -> void = 0;
    virtual auto visit_lambda(LambdaExpr* expr) -> void = 0;
    virtual auto visit_if(IfExpr* expr) -> void = 0;
    virtual auto visit_block(BlockExpr* expr) -> void = 0;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;

    virtual auto visit_let(LetStmt* stmt) -> void = 0;
    virtual auto visit_return(ReturnStmt* stmt) -> void = 0;
    virtual auto visit_expr_stmt(ExprStmt* stmt) -> void = 0;
};

class PatternVisitor {
public:
    virtual ~PatternVisitor() = default;

    virtual auto visit_identifier_pattern(IdentifierPattern* pattern) -> void = 0;
    virtual auto visit_tuple_pattern(TuplePattern* pattern) -> void = 0;
};

class TypeVisitor {
public:
    virtual ~TypeVisitor() = default;

    virtual auto visit_named_type(NamedType* type) -> void = 0;
    virtual auto visit_list_type(ListType* type) -> void = 0;
    virtual auto visit_tuple_type(TupleType* type) -> void = 0;
};

// ===== Utility Functions =====

auto binary_op_from_token(TokenType type) -> std::optional<BinaryOp>;
auto unary_op_from_token(TokenType type) -> std::optional<UnaryOp>;
auto binary_op_name(BinaryOp op) -> std::string_view;
auto unary_op_name(UnaryOp op) -> std::string_view;

} // namespace lucid::ast
