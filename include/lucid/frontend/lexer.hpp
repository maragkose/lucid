#pragma once

#include <lucid/frontend/token.hpp>
#include <string_view>
#include <vector>

namespace lucid {

class Lexer {
public:
    /**
     * Construct a lexer for the given source code.
     * 
     * @param source The source code to tokenize (must outlive the lexer)
     * @param filename The filename for error messages (default: "<input>")
     */
    explicit Lexer(std::string_view source, std::string_view filename = "<input>");
    
    /**
     * Tokenize the entire source and return all tokens.
     * Includes an EOF token at the end.
     * 
     * @return Vector of all tokens
     */
    auto tokenize() -> std::vector<Token>;
    
    /**
     * Get the next token from the source.
     * Can be called repeatedly to stream tokens.
     * Returns EOF token when at end.
     * 
     * @return The next token
     */
    auto next_token() -> Token;
    
    /**
     * Check if the lexer is at the end of source.
     * 
     * @return true if at end, false otherwise
     */
    auto is_at_end() const -> bool;

private:
    std::string_view source_;
    std::string_view filename_;
    size_t current_ = 0;      // Current position in source
    size_t line_ = 1;         // Current line number (1-based)
    size_t line_start_ = 0;   // Byte offset of current line start
    
    // ===== Character access =====
    
    /**
     * Consume and return the current character.
     */
    auto advance() -> char;
    
    /**
     * Look at current character without consuming.
     */
    auto peek() -> char;
    
    /**
     * Look at next character without consuming.
     */
    auto peek_next() -> char;
    
    /**
     * If current character matches expected, consume and return true.
     */
    auto match(char expected) -> bool;
    
    // ===== Token creation =====
    
    /**
     * Create a token from the current scan.
     * Uses start_ to determine lexeme.
     */
    auto make_token(TokenType type) -> Token;
    auto make_token(TokenType type, int64_t value) -> Token;
    auto make_token(TokenType type, double value) -> Token;
    auto make_token(TokenType type, std::string value) -> Token;
    
    /**
     * Create an error token with the given message.
     */
    auto error_token(std::string_view message) -> Token;
    
    /**
     * Get current source location for a token.
     */
    auto current_location(size_t start_offset, size_t length) -> SourceLocation;
    
    // ===== Scanning methods =====
    
    /**
     * Scan a single token from current position.
     */
    auto scan_token() -> Token;
    
    /**
     * Scan a number (integer or float).
     * Current character should be a digit.
     */
    auto scan_number() -> Token;
    
    /**
     * Scan a string literal.
     * Current character should be '"'.
     */
    auto scan_string() -> Token;
    
    /**
     * Scan an identifier or keyword.
     * Current character should be alpha or '_'.
     */
    auto scan_identifier() -> Token;
    
    /**
     * Skip whitespace (spaces, tabs, newlines).
     * Updates line tracking.
     */
    auto skip_whitespace() -> void;
    
    /**
     * Skip a single-line comment (# until newline).
     */
    auto skip_comment() -> void;
    
    /**
     * Skip a multi-line comment (#[ until ]#).
     */
    auto skip_multiline_comment() -> void;
    
    // ===== Character classification =====
    
    static auto is_digit(char c) -> bool;
    static auto is_alpha(char c) -> bool;
    static auto is_alnum(char c) -> bool;
    
    /**
     * Determine if an identifier is a keyword.
     */
    auto identifier_type(std::string_view lexeme) -> TokenType;
    
    // For make_token() to track start of current token
    size_t start_ = 0;
};

} // namespace lucid
