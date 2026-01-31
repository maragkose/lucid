#include <lucid/frontend/parser.hpp>
#include <fmt/format.h>

namespace lucid {

// Operator precedence levels (higher = tighter binding)
namespace {
    constexpr int PREC_LOWEST = 0;
    constexpr int PREC_OR = 10;
    constexpr int PREC_AND = 20;
    constexpr int PREC_COMPARISON = 30;  // ==, !=, <, >, <=, >=
    constexpr int PREC_ADDITIVE = 40;    // +, -
    constexpr int PREC_MULTIPLICATIVE = 50;  // *, /, %
    constexpr int PREC_POWER = 60;       // **
    constexpr int PREC_UNARY = 70;       // not, -, +
    constexpr int PREC_POSTFIX = 80;     // function call, method call, indexing
}

// ===== Constructor =====

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

// ===== Main Parse Method =====

auto Parser::parse() -> ParseResult {
    ParseResult result;

    try {
        result.program = parse_program();
        result.errors = std::move(errors_);
    } catch (...) {
        result.errors = std::move(errors_);
    }

    return result;
}

// ===== Token Stream Management =====

auto Parser::peek() const -> const Token& {
    if (current_ >= tokens_.size()) {
        // Return EOF token
        return tokens_.back();
    }
    return tokens_[current_];
}

auto Parser::peek_ahead(size_t offset) const -> const Token& {
    size_t index = current_ + offset;
    if (index >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[index];
}

auto Parser::previous() const -> const Token& {
    if (current_ == 0) {
        return tokens_[0];
    }
    return tokens_[current_ - 1];
}

auto Parser::advance() -> const Token& {
    if (!is_at_end()) {
        current_++;
    }
    return previous();
}

auto Parser::check(TokenType type) const -> bool {
    if (is_at_end()) return false;
    return peek().type == type;
}

auto Parser::check_any(std::initializer_list<TokenType> types) const -> bool {
    for (auto type : types) {
        if (check(type)) return true;
    }
    return false;
}

auto Parser::match(TokenType type) -> bool {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

auto Parser::match_any(std::initializer_list<TokenType> types) -> bool {
    for (auto type : types) {
        if (match(type)) return true;
    }
    return false;
}

auto Parser::is_at_end() const -> bool {
    return peek().type == TokenType::Eof;
}

auto Parser::expect(TokenType type, std::string_view message) -> bool {
    if (check(type)) {
        advance();
        return true;
    }

    error(std::string(message));
    return false;
}

// ===== Error Handling =====

auto Parser::error(std::string message) -> void {
    error_at(peek(), std::move(message));
}

auto Parser::error_at(const Token& token, std::string message) -> void {
    errors_.emplace_back(token.location, std::move(message));
}

auto Parser::synchronize() -> void {
    // Skip tokens until we find a likely statement boundary
    while (!is_at_end()) {
        // Statement boundaries
        if (check_any({TokenType::Function, TokenType::Let,
                      TokenType::Return, TokenType::If,
                      TokenType::LeftBrace, TokenType::RightBrace})) {
            return;
        }

        advance();
    }
}

// ===== Operator Precedence Helpers =====

auto Parser::get_binary_precedence(TokenType type) const -> int {
    switch (type) {
        case TokenType::Or: return PREC_OR;
        case TokenType::And: return PREC_AND;
        case TokenType::Equal:
        case TokenType::NotEqual:
        case TokenType::Less:
        case TokenType::Greater:
        case TokenType::LessEqual:
        case TokenType::GreaterEqual:
            return PREC_COMPARISON;
        case TokenType::Plus:
        case TokenType::Minus:
            return PREC_ADDITIVE;
        case TokenType::Star:
        case TokenType::Slash:
        case TokenType::Percent:
            return PREC_MULTIPLICATIVE;
        case TokenType::Power:
            return PREC_POWER;
        default:
            return PREC_LOWEST;
    }
}

auto Parser::is_binary_operator(TokenType type) const -> bool {
    return get_binary_precedence(type) > PREC_LOWEST;
}

auto Parser::is_unary_operator(TokenType type) const -> bool {
    return type == TokenType::Not || type == TokenType::Minus || type == TokenType::Plus;
}

auto Parser::is_right_associative(TokenType type) const -> bool {
    // Only ** is right-associative
    return type == TokenType::Power;
}

// ===== Parsing Methods (Stubs for now, to be implemented) =====

auto Parser::parse_program() -> std::unique_ptr<ast::Program> {
    auto start_loc = peek().location;
    std::vector<std::unique_ptr<ast::FunctionDef>> functions;

    // Parse all functions until EOF
    while (!is_at_end()) {
        if (match(TokenType::Function)) {
            auto func = parse_function();
            if (func) {
                functions.push_back(std::move(func));
            } else {
                // Error occurred, try to synchronize
                synchronize();
            }
        } else {
            error("Expected 'function' at top level");
            advance();  // Skip unexpected token to avoid infinite loop
        }
    }

    return std::make_unique<ast::Program>(std::move(functions), start_loc);
}

auto Parser::parse_function() -> std::unique_ptr<ast::FunctionDef> {
    // 'function' already consumed
    auto start_loc = previous().location;

    // Function name
    if (!check(TokenType::Identifier)) {
        error("Expected function name");
        return nullptr;
    }
    auto name = std::string(advance().lexeme);

    // Parameter list: (param1: Type1, param2: Type2, ...)
    if (!expect(TokenType::LeftParen, "Expected '(' after function name")) {
        return nullptr;
    }

    std::vector<std::unique_ptr<ast::Parameter>> parameters;
    if (!check(TokenType::RightParen)) {
        do {
            auto param = parse_parameter();
            if (!param) {
                return nullptr;
            }
            parameters.push_back(std::move(param));
        } while (match(TokenType::Comma));
    }

    if (!expect(TokenType::RightParen, "Expected ')' after parameters")) {
        return nullptr;
    }

    // Return type: returns Type
    if (!expect(TokenType::Returns, "Expected 'returns' after parameters")) {
        return nullptr;
    }

    auto return_type = parse_type();
    if (!return_type) {
        return nullptr;
    }

    // Function body (block expression)
    if (!expect(TokenType::LeftBrace, "Expected '{' for function body")) {
        return nullptr;
    }

    auto body = parse_block_expression();
    if (!body) {
        return nullptr;
    }

    return std::make_unique<ast::FunctionDef>(
        name, std::move(parameters), std::move(return_type),
        std::move(body), start_loc
    );
}

auto Parser::parse_parameter() -> std::unique_ptr<ast::Parameter> {
    auto start_loc = peek().location;

    // Parameter name
    if (!check(TokenType::Identifier)) {
        error("Expected parameter name");
        return nullptr;
    }
    auto name = std::string(advance().lexeme);

    // Type annotation (required for parameters)
    if (!expect(TokenType::Colon, "Expected ':' after parameter name")) {
        return nullptr;
    }

    auto type = parse_type();
    if (!type) return nullptr;

    return std::make_unique<ast::Parameter>(name, std::move(type), start_loc);
}

auto Parser::parse_statement() -> std::unique_ptr<ast::Stmt> {
    // Basic implementation for Day 4 (blocks in lambdas/if)
    // Full implementation in Day 5

    if (check(TokenType::Let)) {
        advance();
        return parse_let_statement();
    }

    if (check(TokenType::Return)) {
        advance();
        return parse_return_statement();
    }

    // Expression statement
    return parse_expression_statement();
}

auto Parser::parse_let_statement() -> std::unique_ptr<ast::LetStmt> {
    // 'let' already consumed
    auto start_loc = previous().location;

    // Parse pattern (identifier or tuple destructuring)
    auto pattern = parse_pattern();
    if (!pattern) return nullptr;

    // Optional type annotation: let x: Int = ...
    std::optional<std::unique_ptr<ast::Type>> type_annotation;
    if (match(TokenType::Colon)) {
        auto type = parse_type();
        if (!type) return nullptr;
        type_annotation = std::move(type);
    }

    // Expect '='
    if (!expect(TokenType::Assign, "Expected '=' in let statement")) {
        return nullptr;
    }

    // Parse initializer expression
    auto initializer = parse_expression();
    if (!initializer) return nullptr;

    return std::make_unique<ast::LetStmt>(
        std::move(pattern), std::move(type_annotation),
        std::move(initializer), start_loc
    );
}

auto Parser::parse_return_statement() -> std::unique_ptr<ast::ReturnStmt> {
    // 'return' already consumed
    auto start_loc = previous().location;

    auto value = parse_expression();
    if (!value) return nullptr;

    return std::make_unique<ast::ReturnStmt>(std::move(value), start_loc);
}

auto Parser::parse_expression_statement() -> std::unique_ptr<ast::ExprStmt> {
    auto expr = parse_expression();
    if (!expr) return nullptr;

    auto start_loc = expr->location;
    return std::make_unique<ast::ExprStmt>(std::move(expr), start_loc);
}

auto Parser::parse_expression() -> std::unique_ptr<ast::Expr> {
    // Parse with precedence climbing
    return parse_precedence(PREC_LOWEST);
}

auto Parser::parse_precedence(int min_prec) -> std::unique_ptr<ast::Expr> {
    // Parse prefix (unary operators or primary)
    auto left = parse_prefix();
    if (!left) return nullptr;

    // Parse postfix operators FIRST (calls, indexing) - they have higher precedence
    left = parse_postfix(std::move(left));
    if (!left) return nullptr;

    // Parse infix operators with precedence
    while (!is_at_end()) {
        // Skip newlines before checking for binary operators
        int skipped = 0;
        while (match(TokenType::Newline)) {
            skipped++;
        }
        // if (skipped > 0) fmt::print("Skipped {} newlines in expression\n", skipped);

        if (!is_binary_operator(peek().type)) {
            break;
        }

        int prec = get_binary_precedence(peek().type);

        if (prec < min_prec) {
            break;
        }

        // Handle associativity
        int next_min_prec = prec;
        if (!is_right_associative(peek().type)) {
            next_min_prec = prec + 1;
        }

        const auto& op_token = advance();

        // Skip newlines after binary operator
        while (match(TokenType::Newline)) {
            // Skip
        }

        auto right = parse_precedence(next_min_prec);
        if (!right) return nullptr;

        auto op = ast::binary_op_from_token(op_token.type);
        if (!op) {
            error_at(op_token, "Invalid binary operator");
            return nullptr;
        }

        left = std::make_unique<ast::BinaryExpr>(
            *op, std::move(left), std::move(right), op_token.location
        );
    }

    return left;
}

auto Parser::parse_prefix() -> std::unique_ptr<ast::Expr> {
    // Handle unary operators
    if (is_unary_operator(peek().type)) {
        const auto& op_token = advance();
        auto operand = parse_precedence(PREC_UNARY);
        if (!operand) return nullptr;

        auto op = ast::unary_op_from_token(op_token.type);
        if (!op) {
            error_at(op_token, "Invalid unary operator");
            return nullptr;
        }

        return std::make_unique<ast::UnaryExpr>(
            *op, std::move(operand), op_token.location
        );
    }

    // Otherwise parse primary expression
    return parse_primary();
}

auto Parser::parse_primary() -> std::unique_ptr<ast::Expr> {
    const auto& token = peek();

    // Integer literal
    if (token.type == TokenType::IntLiteral) {
        advance();
        if (token.value.has_value()) {
            int64_t value = std::get<int64_t>(*token.value);
            return std::make_unique<ast::IntLiteralExpr>(value, token.location);
        } else {
            error_at(token, "Integer literal missing value");
            return nullptr;
        }
    }

    // Float literal
    if (token.type == TokenType::FloatLiteral) {
        advance();
        if (token.value.has_value()) {
            double value = std::get<double>(*token.value);
            return std::make_unique<ast::FloatLiteralExpr>(value, token.location);
        } else {
            error_at(token, "Float literal missing value");
            return nullptr;
        }
    }

    // String literal
    if (token.type == TokenType::StringLiteral) {
        advance();
        if (token.value.has_value()) {
            std::string value = std::get<std::string>(*token.value);
            return std::make_unique<ast::StringLiteralExpr>(std::move(value), token.location);
        } else {
            error_at(token, "String literal missing value");
            return nullptr;
        }
    }

    // Boolean literals
    if (token.type == TokenType::True) {
        advance();
        return std::make_unique<ast::BoolLiteralExpr>(true, token.location);
    }

    if (token.type == TokenType::False) {
        advance();
        return std::make_unique<ast::BoolLiteralExpr>(false, token.location);
    }

    // Identifier
    if (token.type == TokenType::Identifier) {
        advance();
        std::string name(token.lexeme);
        return std::make_unique<ast::IdentifierExpr>(std::move(name), token.location);
    }

    // Parenthesized expression or tuple
    if (token.type == TokenType::LeftParen) {
        advance();
        return parse_tuple_or_grouped();
    }

    // List literal
    if (token.type == TokenType::LeftBracket) {
        advance();
        return parse_list_literal();
    }

    // Lambda expression
    if (token.type == TokenType::Lambda) {
        advance();
        return parse_lambda_expression();
    }

    // If expression
    if (token.type == TokenType::If) {
        advance();
        return parse_if_expression();
    }

    // Block expression
    if (token.type == TokenType::LeftBrace) {
        advance();
        return parse_block_expression();
    }

    // Error: unexpected token - advance to avoid infinite loop
    error(fmt::format("Unexpected token in expression: '{}'", std::string(token.lexeme)));
    advance();
    return nullptr;
}

auto Parser::parse_postfix(std::unique_ptr<ast::Expr> expr) -> std::unique_ptr<ast::Expr> {
    while (true) {
        if (match(TokenType::LeftParen)) {
            // Function call: expr(args)
            auto args = parse_call_arguments();
            if (!expect(TokenType::RightParen, "Expected ')' after arguments")) {
                return nullptr;
            }
            expr = std::make_unique<ast::CallExpr>(
                std::move(expr), std::move(args), previous().location
            );
        } else if (match(TokenType::Dot)) {
            // Method call: expr.method(args)
            if (!check(TokenType::Identifier)) {
                error("Expected method name after '.'");
                return nullptr;
            }
            auto method_name = std::string(advance().lexeme);

            if (match(TokenType::LeftParen)) {
                auto args = parse_call_arguments();
                if (!expect(TokenType::RightParen, "Expected ')' after arguments")) {
                    return nullptr;
                }
                expr = std::make_unique<ast::MethodCallExpr>(
                    std::move(expr), std::move(method_name),
                    std::move(args), previous().location
                );
            } else {
                // Field access would go here in future phases
                error("Field access not yet implemented");
                return nullptr;
            }
        } else if (match(TokenType::LeftBracket)) {
            // Index access: expr[index]
            auto index = parse_expression();
            if (!index) return nullptr;

            if (!expect(TokenType::RightBracket, "Expected ']' after index")) {
                return nullptr;
            }
            expr = std::make_unique<ast::IndexExpr>(
                std::move(expr), std::move(index), previous().location
            );
        } else {
            break;
        }
    }

    return expr;
}

auto Parser::parse_if_expression() -> std::unique_ptr<ast::IfExpr> {
    // 'if' already consumed
    auto start_loc = previous().location;

    // Parse condition
    auto condition = parse_expression();
    if (!condition) return nullptr;

    // Parse then branch (should be a block)
    if (!expect(TokenType::LeftBrace, "Expected '{' after if condition")) {
        return nullptr;
    }

    auto then_branch = parse_block_expression();
    if (!then_branch) return nullptr;

    // Parse optional else branch
    std::optional<std::unique_ptr<ast::Expr>> else_branch;
    if (match(TokenType::Else)) {
        if (check(TokenType::If)) {
            // else if - parse as nested if expression
            advance();
            else_branch = parse_if_expression();
        } else {
            // else block
            if (!expect(TokenType::LeftBrace, "Expected '{' after 'else'")) {
                return nullptr;
            }
            else_branch = parse_block_expression();
        }
    }

    return std::make_unique<ast::IfExpr>(
        std::move(condition), std::move(then_branch),
        std::move(else_branch), start_loc
    );
}

auto Parser::parse_lambda_expression() -> std::unique_ptr<ast::LambdaExpr> {
    // 'lambda' already consumed
    auto start_loc = previous().location;
    std::vector<std::string> params;

    // Parse parameter list (untyped identifiers)
    if (!check(TokenType::Colon)) {
        // Parse first parameter
        if (!check(TokenType::Identifier)) {
            error("Expected parameter name after 'lambda'");
            return nullptr;
        }
        params.push_back(std::string(advance().lexeme));

        // Parse remaining parameters
        while (match(TokenType::Comma)) {
            if (!check(TokenType::Identifier)) {
                error("Expected parameter name after ','");
                return nullptr;
            }
            params.push_back(std::string(advance().lexeme));
        }
    }

    if (!expect(TokenType::Colon, "Expected ':' after lambda parameters")) {
        return nullptr;
    }

    // Parse body (expression or block)
    std::unique_ptr<ast::Expr> body;
    if (check(TokenType::LeftBrace)) {
        // Block body
        advance();
        body = parse_block_expression();
    } else {
        // Expression body
        body = parse_expression();
    }

    if (!body) return nullptr;

    return std::make_unique<ast::LambdaExpr>(
        std::move(params), std::move(body), start_loc
    );
}

auto Parser::parse_block_expression() -> std::unique_ptr<ast::BlockExpr> {
    // '{' already consumed
    auto start_loc = previous().location;
    std::vector<std::unique_ptr<ast::Stmt>> statements;

    // Skip any leading newlines
    while (match(TokenType::Newline)) {
        // Skip newlines
    }

    // Parse statements until '}'
    while (!check(TokenType::RightBrace) && !is_at_end()) {
        auto stmt = parse_statement();
        if (!stmt) {
            // Error recovery: synchronize to next statement
            synchronize();
            continue;
        }
        statements.push_back(std::move(stmt));

        // Skip newlines after each statement
        while (match(TokenType::Newline)) {
            // Skip newlines
        }
    }

    if (!expect(TokenType::RightBrace, "Expected '}' after block")) {
        return nullptr;
    }

    return std::make_unique<ast::BlockExpr>(std::move(statements), start_loc);
}

auto Parser::parse_tuple_or_grouped() -> std::unique_ptr<ast::Expr> {
    // '(' already consumed
    auto start_loc = previous().location;

    // Empty tuple: ()
    if (check(TokenType::RightParen)) {
        advance();
        return std::make_unique<ast::TupleExpr>(
            std::vector<std::unique_ptr<ast::Expr>>{}, start_loc
        );
    }

    // Parse first expression
    auto first = parse_expression();
    if (!first) return nullptr;

    // Check if it's a tuple or grouped expression
    if (match(TokenType::Comma)) {
        // Tuple: (expr, ...)
        std::vector<std::unique_ptr<ast::Expr>> elements;
        elements.push_back(std::move(first));

        // Parse remaining elements
        do {
            // Allow trailing comma
            if (check(TokenType::RightParen)) break;

            auto elem = parse_expression();
            if (!elem) return nullptr;
            elements.push_back(std::move(elem));
        } while (match(TokenType::Comma));

        if (!expect(TokenType::RightParen, "Expected ')' after tuple elements")) {
            return nullptr;
        }

        return std::make_unique<ast::TupleExpr>(std::move(elements), start_loc);
    } else {
        // Grouped expression: (expr)
        if (!expect(TokenType::RightParen, "Expected ')' after expression")) {
            return nullptr;
        }
        return first;
    }
}

auto Parser::parse_list_literal() -> std::unique_ptr<ast::ListExpr> {
    // '[' already consumed
    auto start_loc = previous().location;
    std::vector<std::unique_ptr<ast::Expr>> elements;

    // Empty list: []
    if (check(TokenType::RightBracket)) {
        advance();
        return std::make_unique<ast::ListExpr>(std::move(elements), start_loc);
    }

    // Parse first element
    auto elem = parse_expression();
    if (!elem) return nullptr;
    elements.push_back(std::move(elem));

    // Parse remaining elements
    while (match(TokenType::Comma)) {
        // Allow trailing comma
        if (check(TokenType::RightBracket)) break;

        elem = parse_expression();
        if (!elem) return nullptr;
        elements.push_back(std::move(elem));
    }

    if (!expect(TokenType::RightBracket, "Expected ']' after list elements")) {
        return nullptr;
    }

    return std::make_unique<ast::ListExpr>(std::move(elements), start_loc);
}

auto Parser::parse_call_arguments() -> std::vector<std::unique_ptr<ast::Expr>> {
    std::vector<std::unique_ptr<ast::Expr>> args;

    // Empty argument list
    if (check(TokenType::RightParen)) {
        return args;
    }

    // Parse first argument
    auto arg = parse_expression();
    if (!arg) return args;
    args.push_back(std::move(arg));

    // Parse remaining arguments
    while (match(TokenType::Comma)) {
        arg = parse_expression();
        if (!arg) return args;
        args.push_back(std::move(arg));
    }

    return args;
}

auto Parser::parse_type() -> std::unique_ptr<ast::Type> {
    return parse_primary_type();
}

auto Parser::parse_primary_type() -> std::unique_ptr<ast::Type> {
    auto start_loc = peek().location;

    // Built-in type keywords: Int, Float, String, Bool
    if (check(TokenType::TypeInt)) {
        advance();
        return std::make_unique<ast::NamedType>("Int", start_loc);
    }
    if (check(TokenType::TypeFloat)) {
        advance();
        return std::make_unique<ast::NamedType>("Float", start_loc);
    }
    if (check(TokenType::TypeString)) {
        advance();
        return std::make_unique<ast::NamedType>("String", start_loc);
    }
    if (check(TokenType::TypeBool)) {
        advance();
        return std::make_unique<ast::NamedType>("Bool", start_loc);
    }

    // List type keyword with element type: List[T]
    if (check(TokenType::TypeList)) {
        advance();
        if (!expect(TokenType::LeftBracket, "Expected '[' after 'List'")) {
            return nullptr;
        }
        auto element_type = parse_type();
        if (!element_type) return nullptr;

        if (!expect(TokenType::RightBracket, "Expected ']' after list element type")) {
            return nullptr;
        }

        return std::make_unique<ast::ListType>(std::move(element_type), start_loc);
    }

    // Named type: custom types (identifiers)
    if (check(TokenType::Identifier)) {
        auto name = std::string(advance().lexeme);

        // Check for Generic[T] syntax
        if (match(TokenType::LeftBracket)) {
            auto element_type = parse_type();
            if (!element_type) return nullptr;

            if (!expect(TokenType::RightBracket, "Expected ']' after type parameter")) {
                return nullptr;
            }

            // For now, treat as ListType (can expand later for generic types)
            return std::make_unique<ast::ListType>(std::move(element_type), start_loc);
        }

        // Simple named type
        return std::make_unique<ast::NamedType>(name, start_loc);
    }

    // Tuple type: (T1, T2, ...)
    if (match(TokenType::LeftParen)) {
        std::vector<std::unique_ptr<ast::Type>> element_types;

        // Empty tuple type: ()
        if (match(TokenType::RightParen)) {
            return std::make_unique<ast::TupleType>(std::move(element_types), start_loc);
        }

        // Parse types
        do {
            auto type = parse_type();
            if (!type) return nullptr;
            element_types.push_back(std::move(type));
        } while (match(TokenType::Comma));

        if (!expect(TokenType::RightParen, "Expected ')' after tuple type")) {
            return nullptr;
        }

        return std::make_unique<ast::TupleType>(std::move(element_types), start_loc);
    }

    error("Expected type");
    return nullptr;
}

auto Parser::parse_pattern() -> std::unique_ptr<ast::Pattern> {
    auto start_loc = peek().location;

    // Identifier pattern: x
    if (check(TokenType::Identifier)) {
        auto name = std::string(advance().lexeme);
        return std::make_unique<ast::IdentifierPattern>(name, start_loc);
    }

    // Tuple destructuring pattern: (x, y, ...)
    if (match(TokenType::LeftParen)) {
        std::vector<std::unique_ptr<ast::Pattern>> patterns;

        // Empty tuple pattern: ()
        if (match(TokenType::RightParen)) {
            return std::make_unique<ast::TuplePattern>(std::move(patterns), start_loc);
        }

        // Parse patterns
        do {
            auto pattern = parse_pattern();
            if (!pattern) return nullptr;
            patterns.push_back(std::move(pattern));
        } while (match(TokenType::Comma));

        if (!expect(TokenType::RightParen, "Expected ')' after tuple pattern")) {
            return nullptr;
        }

        return std::make_unique<ast::TuplePattern>(std::move(patterns), start_loc);
    }

    error("Expected pattern (identifier or tuple)");
    return nullptr;
}

// ===== Convenience Function =====

auto parse_source(std::string_view source, std::string_view filename) -> ParseResult {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

} // namespace lucid
