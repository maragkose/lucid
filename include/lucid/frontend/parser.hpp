#pragma once

#include <lucid/frontend/ast.hpp>
#include <lucid/frontend/lexer.hpp>
#include <lucid/frontend/token.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lucid {

// ===== Parse Error =====

struct ParseError {
    SourceLocation location;
    std::string message;

    ParseError(SourceLocation location, std::string message)
        : location(location), message(std::move(message)) {}
};

// ===== Parse Result =====

class ParseResult {
public:
    std::optional<std::unique_ptr<ast::Program>> program;
    std::vector<ParseError> errors;

    auto has_errors() const -> bool { return !errors.empty(); }
    auto is_ok() const -> bool { return !has_errors() && program.has_value(); }
};

// ===== Parser =====

class Parser {
public:
    /**
     * Construct a parser from a token stream.
     *
     * @param tokens Token vector (must include EOF token at end)
     */
    explicit Parser(std::vector<Token> tokens);

    /**
     * Parse the token stream into an AST.
     * Returns a ParseResult with either a program or errors.
     *
     * @return ParseResult with program or errors
     */
    auto parse() -> ParseResult;

    /**
     * Parse a single expression (useful for testing).
     * Returns nullptr on error.
     */
    auto parse_expression() -> std::unique_ptr<ast::Expr>;

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::vector<ParseError> errors_;

    // ===== Token Stream Management =====

    /**
     * Get current token without consuming.
     */
    auto peek() const -> const Token&;

    /**
     * Get token at offset from current without consuming.
     */
    auto peek_ahead(size_t offset) const -> const Token&;

    /**
     * Get previous token (already consumed).
     */
    auto previous() const -> const Token&;

    /**
     * Consume and return current token.
     */
    auto advance() -> const Token&;

    /**
     * Check if current token matches type.
     */
    auto check(TokenType type) const -> bool;

    /**
     * Check if current token matches any of the given types.
     */
    auto check_any(std::initializer_list<TokenType> types) const -> bool;

    /**
     * If current token matches, consume and return true.
     */
    auto match(TokenType type) -> bool;

    /**
     * If current token matches any type, consume and return true.
     */
    auto match_any(std::initializer_list<TokenType> types) -> bool;

    /**
     * Check if at end of token stream.
     */
    auto is_at_end() const -> bool;

    /**
     * Consume token and verify it matches expected type.
     * If not, record error and return false.
     */
    auto expect(TokenType type, std::string_view message) -> bool;

    // ===== Error Handling =====

    /**
     * Record a parse error at current location.
     */
    auto error(std::string message) -> void;

    /**
     * Record a parse error at specific location.
     */
    auto error_at(const Token& token, std::string message) -> void;

    /**
     * Synchronize after error to recover parsing.
     * Skips tokens until a statement boundary.
     */
    auto synchronize() -> void;

    // ===== Parsing Methods =====

    // Top-level
    auto parse_program() -> std::unique_ptr<ast::Program>;
    auto parse_function() -> std::unique_ptr<ast::FunctionDef>;
    auto parse_parameter() -> std::unique_ptr<ast::Parameter>;

    // Statements
    auto parse_statement() -> std::unique_ptr<ast::Stmt>;
    auto parse_let_statement() -> std::unique_ptr<ast::LetStmt>;
    auto parse_return_statement() -> std::unique_ptr<ast::ReturnStmt>;
    auto parse_expression_statement() -> std::unique_ptr<ast::ExprStmt>;

    // Expressions (parse_expression is public for testing)
    auto parse_precedence(int min_prec) -> std::unique_ptr<ast::Expr>;
    auto parse_prefix() -> std::unique_ptr<ast::Expr>;
    auto parse_primary() -> std::unique_ptr<ast::Expr>;
    auto parse_postfix(std::unique_ptr<ast::Expr> expr) -> std::unique_ptr<ast::Expr>;

    // Specific expressions
    auto parse_if_expression() -> std::unique_ptr<ast::IfExpr>;
    auto parse_lambda_expression() -> std::unique_ptr<ast::LambdaExpr>;
    auto parse_block_expression() -> std::unique_ptr<ast::BlockExpr>;
    auto parse_tuple_or_grouped() -> std::unique_ptr<ast::Expr>;
    auto parse_list_literal() -> std::unique_ptr<ast::ListExpr>;
    auto parse_call_arguments() -> std::vector<std::unique_ptr<ast::Expr>>;

    // Types
    auto parse_type() -> std::unique_ptr<ast::Type>;
    auto parse_primary_type() -> std::unique_ptr<ast::Type>;

    // Patterns
    auto parse_pattern() -> std::unique_ptr<ast::Pattern>;

    // Operator precedence helpers
    auto get_binary_precedence(TokenType type) const -> int;
    auto is_binary_operator(TokenType type) const -> bool;
    auto is_unary_operator(TokenType type) const -> bool;
    auto is_right_associative(TokenType type) const -> bool;
};

// ===== Convenience Functions =====

/**
 * Parse source code string into AST.
 * Handles lexing and parsing in one call.
 */
auto parse_source(std::string_view source, std::string_view filename = "<input>")
    -> ParseResult;

} // namespace lucid
