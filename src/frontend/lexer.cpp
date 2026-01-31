#include <lucid/frontend/lexer.hpp>
#include <lucid/frontend/token.hpp>
#include <charconv>
#include <unordered_map>
#include <string>
#include <cctype>

namespace lucid {

// ===== Token type name utility =====

auto token_type_name(TokenType type) -> std::string_view {
    switch (type) {
        case TokenType::Function: return "Function";
        case TokenType::Returns: return "Returns";
        case TokenType::Let: return "Let";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::Return: return "Return";
        case TokenType::Lambda: return "Lambda";
        case TokenType::TypeInt: return "TypeInt";
        case TokenType::TypeFloat: return "TypeFloat";
        case TokenType::TypeString: return "TypeString";
        case TokenType::TypeBool: return "TypeBool";
        case TokenType::TypeList: return "TypeList";
        case TokenType::IntLiteral: return "IntLiteral";
        case TokenType::FloatLiteral: return "FloatLiteral";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Percent: return "Percent";
        case TokenType::Power: return "Power";
        case TokenType::Equal: return "Equal";
        case TokenType::NotEqual: return "NotEqual";
        case TokenType::Less: return "Less";
        case TokenType::Greater: return "Greater";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::And: return "And";
        case TokenType::Or: return "Or";
        case TokenType::Not: return "Not";
        case TokenType::Assign: return "Assign";
        case TokenType::Colon: return "Colon";
        case TokenType::Dot: return "Dot";
        case TokenType::Comma: return "Comma";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Newline: return "Newline";
        case TokenType::Eof: return "Eof";
        case TokenType::Error: return "Error";
        default: return "Unknown";
    }
}

// ===== Lexer implementation =====

Lexer::Lexer(std::string_view source, std::string_view filename)
    : source_(source), filename_(filename) {}

auto Lexer::tokenize() -> std::vector<Token> {
    std::vector<Token> tokens;

    while (!is_at_end()) {
        Token token = next_token();
        tokens.push_back(token);

        if (token.type == TokenType::Eof || token.type == TokenType::Error) {
            break;
        }
    }

    // Ensure we have an EOF token at the end
    if (tokens.empty() || tokens.back().type != TokenType::Eof) {
        tokens.push_back(make_token(TokenType::Eof));
    }

    return tokens;
}

auto Lexer::next_token() -> Token {
    if (is_at_end()) {
        return make_token(TokenType::Eof);
    }

    return scan_token();
}

auto Lexer::is_at_end() const -> bool {
    return current_ >= source_.length();
}

// ===== Character access =====

auto Lexer::advance() -> char {
    if (is_at_end()) return '\0';
    return source_[current_++];
}

auto Lexer::peek() -> char {
    if (is_at_end()) return '\0';
    return source_[current_];
}

auto Lexer::peek_next() -> char {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

auto Lexer::match(char expected) -> bool {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    return true;
}

// ===== Token creation =====

auto Lexer::make_token(TokenType type) -> Token {
    size_t length = current_ - start_;
    std::string_view lexeme = source_.substr(start_, length);
    size_t column = start_ - line_start_ + 1;

    SourceLocation loc(filename_, line_, column, start_, length);
    return Token(type, lexeme, loc);
}

auto Lexer::make_token(TokenType type, int64_t value) -> Token {
    size_t length = current_ - start_;
    std::string_view lexeme = source_.substr(start_, length);
    size_t column = start_ - line_start_ + 1;

    SourceLocation loc(filename_, line_, column, start_, length);
    return Token(type, lexeme, loc, value);
}

auto Lexer::make_token(TokenType type, double value) -> Token {
    size_t length = current_ - start_;
    std::string_view lexeme = source_.substr(start_, length);
    size_t column = start_ - line_start_ + 1;

    SourceLocation loc(filename_, line_, column, start_, length);
    return Token(type, lexeme, loc, value);
}

auto Lexer::make_token(TokenType type, std::string value) -> Token {
    size_t length = current_ - start_;
    std::string_view lexeme = source_.substr(start_, length);
    size_t column = start_ - line_start_ + 1;

    SourceLocation loc(filename_, line_, column, start_, length);
    return Token(type, lexeme, loc, std::move(value));
}

auto Lexer::error_token(std::string_view message) -> Token {
    size_t length = current_ - start_;
    size_t column = start_ - line_start_ + 1;

    SourceLocation loc(filename_, line_, column, start_, length);
    return Token(TokenType::Error, "", loc, std::string(message));
}

auto Lexer::current_location(size_t start_offset, size_t length) -> SourceLocation {
    size_t column = start_offset - line_start_ + 1;
    return SourceLocation(filename_, line_, column, start_offset, length);
}

// ===== Whitespace and comments =====

auto Lexer::skip_whitespace() -> void {
    while (!is_at_end()) {
        char c = peek();

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;

            case '\n':
                advance();
                line_++;
                line_start_ = current_;
                break;

            default:
                return;
        }
    }
}

auto Lexer::skip_comment() -> void {
    // Skip until end of line
    while (!is_at_end() && peek() != '\n') {
        advance();
    }
}

auto Lexer::skip_multiline_comment() -> void {
    // Already consumed '#['
    int depth = 1;  // For potential nesting support later

    while (!is_at_end() && depth > 0) {
        if (peek() == ']' && peek_next() == '#') {
            advance();  // consume ']'
            advance();  // consume '#'
            depth--;
        } else if (peek() == '\n') {
            advance();
            line_++;
            line_start_ = current_;
        } else {
            advance();
        }
    }
}

// ===== Character classification =====

auto Lexer::is_digit(char c) -> bool {
    return c >= '0' && c <= '9';
}

auto Lexer::is_alpha(char c) -> bool {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

auto Lexer::is_alnum(char c) -> bool {
    return is_alpha(c) || is_digit(c);
}

// ===== Keyword recognition =====

auto Lexer::identifier_type(std::string_view lexeme) -> TokenType {
    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"function", TokenType::Function},
        {"returns", TokenType::Returns},
        {"let", TokenType::Let},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"return", TokenType::Return},
        {"lambda", TokenType::Lambda},
        {"Int", TokenType::TypeInt},
        {"Float", TokenType::TypeFloat},
        {"String", TokenType::TypeString},
        {"Bool", TokenType::TypeBool},
        {"List", TokenType::TypeList},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"and", TokenType::And},
        {"or", TokenType::Or},
        {"not", TokenType::Not},
    };

    if (auto it = keywords.find(lexeme); it != keywords.end()) {
        return it->second;
    }

    return TokenType::Identifier;
}

// ===== Scanning methods =====

auto Lexer::scan_identifier() -> Token {
    // Scan [a-zA-Z_][a-zA-Z0-9_]*
    while (is_alnum(peek()) || peek() == '_') {
        advance();
    }

    std::string_view lexeme = source_.substr(start_, current_ - start_);
    TokenType type = identifier_type(lexeme);

    return make_token(type);
}

auto Lexer::scan_number() -> Token {
    // Scan integer part, allowing underscores
    while (is_digit(peek()) || peek() == '_') {
        advance();
    }

    bool is_float = false;

    // Check for decimal point
    if (peek() == '.' && is_digit(peek_next())) {
        is_float = true;
        advance();  // consume '.'

        while (is_digit(peek()) || peek() == '_') {
            advance();
        }
    }

    // Check for exponent
    if (peek() == 'e' || peek() == 'E') {
        is_float = true;
        advance();  // consume 'e' or 'E'

        if (peek() == '+' || peek() == '-') {
            advance();  // consume sign
        }

        if (!is_digit(peek())) {
            return error_token("Invalid exponent in number literal");
        }

        while (is_digit(peek()) || peek() == '_') {
            advance();
        }
    }

    // Get the lexeme and remove underscores for parsing
    std::string_view lexeme = source_.substr(start_, current_ - start_);
    std::string cleaned;
    cleaned.reserve(lexeme.length());

    for (char c : lexeme) {
        if (c != '_') {
            cleaned += c;
        }
    }

    if (is_float) {
        // Parse as double
        double value;
        auto [ptr, ec] = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value);

        if (ec != std::errc{}) {
            return error_token("Invalid float literal");
        }

        return make_token(TokenType::FloatLiteral, value);
    } else {
        // Parse as int64_t
        int64_t value;
        auto [ptr, ec] = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), value);

        if (ec != std::errc{}) {
            return error_token("Invalid integer literal");
        }

        return make_token(TokenType::IntLiteral, value);
    }
}

auto Lexer::scan_string() -> Token {
    // Already consumed opening '"'
    std::string value;

    while (!is_at_end() && peek() != '"') {
        if (peek() == '\n') {
            line_++;
            line_start_ = current_ + 1;
            value += advance();
        } else if (peek() == '\\') {
            advance();  // consume '\'

            if (is_at_end()) {
                return error_token("Unterminated string literal");
            }

            char c = advance();

            switch (c) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default:
                    // Unknown escape sequence - just include it literally
                    value += '\\';
                    value += c;
                    break;
            }
        } else {
            value += advance();
        }
    }

    if (is_at_end()) {
        return error_token("Unterminated string literal");
    }

    // Consume closing '"'
    advance();

    return make_token(TokenType::StringLiteral, std::move(value));
}

auto Lexer::scan_token() -> Token {
    skip_whitespace();

    if (is_at_end()) {
        return make_token(TokenType::Eof);
    }

    start_ = current_;
    char c = advance();

    // Single character tokens
    switch (c) {
        case '(': return make_token(TokenType::LeftParen);
        case ')': return make_token(TokenType::RightParen);
        case '{': return make_token(TokenType::LeftBrace);
        case '}': return make_token(TokenType::RightBrace);
        case '[': return make_token(TokenType::LeftBracket);
        case ']': return make_token(TokenType::RightBracket);
        case ',': return make_token(TokenType::Comma);
        case '.': return make_token(TokenType::Dot);
        case ':': return make_token(TokenType::Colon);
        case '+': return make_token(TokenType::Plus);
        case '-': return make_token(TokenType::Minus);
        case '%': return make_token(TokenType::Percent);
        case '/': return make_token(TokenType::Slash);

        // Multi-character operators
        case '*':
            if (match('*')) {
                return make_token(TokenType::Power);
            }
            return make_token(TokenType::Star);

        case '=':
            if (match('=')) {
                return make_token(TokenType::Equal);
            }
            return make_token(TokenType::Assign);

        case '!':
            if (match('=')) {
                return make_token(TokenType::NotEqual);
            }
            return error_token("Unexpected character '!'");

        case '<':
            if (match('=')) {
                return make_token(TokenType::LessEqual);
            }
            return make_token(TokenType::Less);

        case '>':
            if (match('=')) {
                return make_token(TokenType::GreaterEqual);
            }
            return make_token(TokenType::Greater);

        // String literal
        case '"':
            return scan_string();

        // Comment
        case '#':
            if (match('[')) {
                skip_multiline_comment();
                return scan_token();  // Recursively get next token
            }
            skip_comment();
            return scan_token();
    }

    // Number literal
    if (is_digit(c)) {
        return scan_number();
    }

    // Identifier or keyword
    if (is_alpha(c) || c == '_') {
        return scan_identifier();
    }

    // Unexpected character
    std::string error_msg = "Unexpected character: '";
    error_msg += c;
    error_msg += "'";
    return error_token(error_msg);
}

} // namespace lucid
