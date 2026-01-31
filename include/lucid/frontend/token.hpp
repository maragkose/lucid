#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace lucid {

enum class TokenType {
    // Keywords
    Function,
    Returns,
    Let,
    If,
    Else,
    Return,
    Lambda,
    
    // Type names
    TypeInt,
    TypeFloat,
    TypeString,
    TypeBool,
    TypeList,
    
    // Literals
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    True,
    False,
    
    // Operators
    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %
    Power,         // **
    
    // Comparison
    Equal,         // ==
    NotEqual,      // !=
    Less,          // <
    Greater,       // >
    LessEqual,     // <=
    GreaterEqual,  // >=
    
    // Logical
    And,           // and
    Or,            // or
    Not,           // not
    
    // Punctuation
    Assign,        // =
    Colon,         // :
    Dot,           // .
    Comma,         // ,
    LeftParen,     // (
    RightParen,    // )
    LeftBrace,     // {
    RightBrace,    // }
    LeftBracket,   // [
    RightBracket,  // ]
    
    // Special
    Identifier,
    Newline,
    Eof,
    Error
};

struct SourceLocation {
    std::string_view filename;
    size_t line;
    size_t column;
    size_t offset;    // Byte offset in source
    size_t length;    // Length of token in bytes
    
    SourceLocation(std::string_view filename, size_t line, size_t column,
                   size_t offset, size_t length)
        : filename(filename), line(line), column(column),
          offset(offset), length(length) {}
};

struct Token {
    TokenType type;
    std::string_view lexeme;  // Zero-copy: points into source buffer
    SourceLocation location;
    
    // For literals that need parsed values
    // - IntLiteral: int64_t
    // - FloatLiteral: double
    // - StringLiteral: std::string (after escape processing)
    // - Error: std::string (error message)
    std::optional<std::variant<int64_t, double, std::string>> value;
    
    // Constructors
    Token(TokenType type, std::string_view lexeme, SourceLocation location)
        : type(type), lexeme(lexeme), location(location) {}
    
    Token(TokenType type, std::string_view lexeme, SourceLocation location, int64_t val)
        : type(type), lexeme(lexeme), location(location), value(val) {}
    
    Token(TokenType type, std::string_view lexeme, SourceLocation location, double val)
        : type(type), lexeme(lexeme), location(location), value(val) {}
    
    Token(TokenType type, std::string_view lexeme, SourceLocation location, std::string val)
        : type(type), lexeme(lexeme), location(location), value(std::move(val)) {}
};

// Utility function to get token type name for debugging/error messages
auto token_type_name(TokenType type) -> std::string_view;

} // namespace lucid
