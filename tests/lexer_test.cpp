#include <lucid/frontend/lexer.hpp>

// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include <catch_amalgamated.hpp>
#endif

#include <string>
#include <vector>

using namespace lucid;

TEST_CASE("Lexer: Empty source", "[lexer]") {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].type == TokenType::Eof);
}

TEST_CASE("Lexer: Keywords", "[lexer]") {
    Lexer lexer("function returns let if else return lambda");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens.size() == 8);  // 7 keywords + EOF
    REQUIRE(tokens[0].type == TokenType::Function);
    REQUIRE(tokens[1].type == TokenType::Returns);
    REQUIRE(tokens[2].type == TokenType::Let);
    REQUIRE(tokens[3].type == TokenType::If);
    REQUIRE(tokens[4].type == TokenType::Else);
    REQUIRE(tokens[5].type == TokenType::Return);
    REQUIRE(tokens[6].type == TokenType::Lambda);
    REQUIRE(tokens[7].type == TokenType::Eof);
}

TEST_CASE("Lexer: Type names", "[lexer]") {
    Lexer lexer("Int Float String Bool List");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens.size() == 6);  // 5 types + EOF
    REQUIRE(tokens[0].type == TokenType::TypeInt);
    REQUIRE(tokens[1].type == TokenType::TypeFloat);
    REQUIRE(tokens[2].type == TokenType::TypeString);
    REQUIRE(tokens[3].type == TokenType::TypeBool);
    REQUIRE(tokens[4].type == TokenType::TypeList);
}

TEST_CASE("Lexer: Boolean literals", "[lexer]") {
    Lexer lexer("true false");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens.size() == 3);  // 2 bools + EOF
    REQUIRE(tokens[0].type == TokenType::True);
    REQUIRE(tokens[1].type == TokenType::False);
}

TEST_CASE("Lexer: Integer literals", "[lexer]") {
    Lexer lexer("0 42 -17 1000 1_000_000");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::IntLiteral);
    REQUIRE(tokens[0].lexeme == "0");
    REQUIRE(std::get<int64_t>(*tokens[0].value) == 0);
    
    REQUIRE(tokens[1].type == TokenType::IntLiteral);
    REQUIRE(std::get<int64_t>(*tokens[1].value) == 42);
    
    REQUIRE(tokens[2].type == TokenType::Minus);
    REQUIRE(tokens[3].type == TokenType::IntLiteral);
    REQUIRE(std::get<int64_t>(*tokens[3].value) == 17);
    
    REQUIRE(tokens[4].type == TokenType::IntLiteral);
    REQUIRE(std::get<int64_t>(*tokens[4].value) == 1000);
    
    REQUIRE(tokens[5].type == TokenType::IntLiteral);
    REQUIRE(tokens[5].lexeme == "1_000_000");
    REQUIRE(std::get<int64_t>(*tokens[5].value) == 1000000);
}

TEST_CASE("Lexer: Float literals", "[lexer]") {
    Lexer lexer("3.14 0.5 1.5e10 2.5e-3");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::FloatLiteral);
    REQUIRE(std::get<double>(*tokens[0].value) == 3.14);
    
    REQUIRE(tokens[1].type == TokenType::FloatLiteral);
    REQUIRE(std::get<double>(*tokens[1].value) == 0.5);
    
    REQUIRE(tokens[2].type == TokenType::FloatLiteral);
    REQUIRE(std::get<double>(*tokens[2].value) == 1.5e10);
    
    REQUIRE(tokens[3].type == TokenType::FloatLiteral);
    REQUIRE(std::get<double>(*tokens[3].value) == 2.5e-3);
}

TEST_CASE("Lexer: String literals", "[lexer]") {
    Lexer lexer(R"("hello" "world" "with\nnewline")");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::StringLiteral);
    REQUIRE(std::get<std::string>(*tokens[0].value) == "hello");
    
    REQUIRE(tokens[1].type == TokenType::StringLiteral);
    REQUIRE(std::get<std::string>(*tokens[1].value) == "world");
    
    REQUIRE(tokens[2].type == TokenType::StringLiteral);
    REQUIRE(std::get<std::string>(*tokens[2].value) == "with\nnewline");
}

TEST_CASE("Lexer: Identifiers", "[lexer]") {
    Lexer lexer("foo bar_baz _internal x123");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Identifier);
    REQUIRE(tokens[0].lexeme == "foo");
    
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[1].lexeme == "bar_baz");
    
    REQUIRE(tokens[2].type == TokenType::Identifier);
    REQUIRE(tokens[2].lexeme == "_internal");
    
    REQUIRE(tokens[3].type == TokenType::Identifier);
    REQUIRE(tokens[3].lexeme == "x123");
}

TEST_CASE("Lexer: Operators", "[lexer]") {
    Lexer lexer("+ - * / % **");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Plus);
    REQUIRE(tokens[1].type == TokenType::Minus);
    REQUIRE(tokens[2].type == TokenType::Star);
    REQUIRE(tokens[3].type == TokenType::Slash);
    REQUIRE(tokens[4].type == TokenType::Percent);
    REQUIRE(tokens[5].type == TokenType::Power);
}

TEST_CASE("Lexer: Comparison operators", "[lexer]") {
    Lexer lexer("== != < > <= >=");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Equal);
    REQUIRE(tokens[1].type == TokenType::NotEqual);
    REQUIRE(tokens[2].type == TokenType::Less);
    REQUIRE(tokens[3].type == TokenType::Greater);
    REQUIRE(tokens[4].type == TokenType::LessEqual);
    REQUIRE(tokens[5].type == TokenType::GreaterEqual);
}

TEST_CASE("Lexer: Logical operators", "[lexer]") {
    Lexer lexer("and or not");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::And);
    REQUIRE(tokens[1].type == TokenType::Or);
    REQUIRE(tokens[2].type == TokenType::Not);
}

TEST_CASE("Lexer: Punctuation", "[lexer]") {
    Lexer lexer("= : . , ( ) { } [ ]");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Assign);
    REQUIRE(tokens[1].type == TokenType::Colon);
    REQUIRE(tokens[2].type == TokenType::Dot);
    REQUIRE(tokens[3].type == TokenType::Comma);
    REQUIRE(tokens[4].type == TokenType::LeftParen);
    REQUIRE(tokens[5].type == TokenType::RightParen);
    REQUIRE(tokens[6].type == TokenType::LeftBrace);
    REQUIRE(tokens[7].type == TokenType::RightBrace);
    REQUIRE(tokens[8].type == TokenType::LeftBracket);
    REQUIRE(tokens[9].type == TokenType::RightBracket);
}

TEST_CASE("Lexer: Single-line comments", "[lexer]") {
    Lexer lexer(R"(
        # This is a comment
        let x = 42
        # Another comment
    )");
    auto tokens = lexer.tokenize();
    
    // Comments should be skipped
    REQUIRE(tokens[0].type == TokenType::Let);
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[2].type == TokenType::Assign);
    REQUIRE(tokens[3].type == TokenType::IntLiteral);
}

TEST_CASE("Lexer: Multi-line comments", "[lexer]") {
    Lexer lexer(R"(
        let x = 1
        #[ This is
           a multi-line
           comment ]#
        let y = 2
    )");
    auto tokens = lexer.tokenize();
    
    // Find the identifiers
    bool found_x = false;
    bool found_y = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::Identifier) {
            if (token.lexeme == "x") found_x = true;
            if (token.lexeme == "y") found_y = true;
        }
    }
    
    REQUIRE(found_x);
    REQUIRE(found_y);
}

TEST_CASE("Lexer: Complete function", "[lexer]") {
    Lexer lexer(R"(
        function add(x: Int, y: Int) returns Int {
            return x + y
        }
    )");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Function);
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[1].lexeme == "add");
    REQUIRE(tokens[2].type == TokenType::LeftParen);
    REQUIRE(tokens[3].type == TokenType::Identifier);
    REQUIRE(tokens[3].lexeme == "x");
    REQUIRE(tokens[4].type == TokenType::Colon);
    REQUIRE(tokens[5].type == TokenType::TypeInt);
    REQUIRE(tokens[6].type == TokenType::Comma);
    REQUIRE(tokens[7].type == TokenType::Identifier);
    REQUIRE(tokens[7].lexeme == "y");
    REQUIRE(tokens[8].type == TokenType::Colon);
    REQUIRE(tokens[9].type == TokenType::TypeInt);
    REQUIRE(tokens[10].type == TokenType::RightParen);
    REQUIRE(tokens[11].type == TokenType::Returns);
    REQUIRE(tokens[12].type == TokenType::TypeInt);
    REQUIRE(tokens[13].type == TokenType::LeftBrace);
    REQUIRE(tokens[14].type == TokenType::Return);
    REQUIRE(tokens[15].type == TokenType::Identifier);
    REQUIRE(tokens[15].lexeme == "x");
    REQUIRE(tokens[16].type == TokenType::Plus);
    REQUIRE(tokens[17].type == TokenType::Identifier);
    REQUIRE(tokens[17].lexeme == "y");
    REQUIRE(tokens[18].type == TokenType::RightBrace);
}

TEST_CASE("Lexer: Lambda expression", "[lexer]") {
    Lexer lexer("lambda x: x * 2");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Lambda);
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[2].type == TokenType::Colon);
    REQUIRE(tokens[3].type == TokenType::Identifier);
    REQUIRE(tokens[4].type == TokenType::Star);
    REQUIRE(tokens[5].type == TokenType::IntLiteral);
}

TEST_CASE("Lexer: Location tracking", "[lexer]") {
    Lexer lexer("let x = 42\nlet y = 10", "test.lucid");
    auto tokens = lexer.tokenize();
    
    // First line
    REQUIRE(tokens[0].location.line == 1);
    REQUIRE(tokens[0].location.column == 1);
    REQUIRE(tokens[0].location.filename == "test.lucid");
    
    REQUIRE(tokens[1].location.line == 1);
    REQUIRE(tokens[1].location.column == 5);
    
    // Second line (after newline)
    REQUIRE(tokens[4].location.line == 2);
    REQUIRE(tokens[4].location.column == 1);
}

TEST_CASE("Lexer: Error - unterminated string", "[lexer]") {
    Lexer lexer(R"("unterminated)");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Error);
    REQUIRE(tokens[0].value.has_value());
    // Error message should mention unterminated string
}

TEST_CASE("Lexer: Error - invalid character", "[lexer]") {
    Lexer lexer("let x @ 42");
    auto tokens = lexer.tokenize();
    
    // Should have error token for @
    bool found_error = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::Error) {
            found_error = true;
            break;
        }
    }
    REQUIRE(found_error);
}

TEST_CASE("Lexer: Whitespace handling", "[lexer]") {
    Lexer lexer("  let  \t x  \n  =  \r\n  42  ");
    auto tokens = lexer.tokenize();
    
    REQUIRE(tokens[0].type == TokenType::Let);
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[2].type == TokenType::Assign);
    REQUIRE(tokens[3].type == TokenType::IntLiteral);
}

TEST_CASE("Lexer: Streaming API", "[lexer]") {
    Lexer lexer("let x = 42");
    
    auto tok1 = lexer.next_token();
    REQUIRE(tok1.type == TokenType::Let);
    
    auto tok2 = lexer.next_token();
    REQUIRE(tok2.type == TokenType::Identifier);
    
    auto tok3 = lexer.next_token();
    REQUIRE(tok3.type == TokenType::Assign);
    
    auto tok4 = lexer.next_token();
    REQUIRE(tok4.type == TokenType::IntLiteral);
    
    auto tok5 = lexer.next_token();
    REQUIRE(tok5.type == TokenType::Eof);
    
    // Should keep returning EOF
    auto tok6 = lexer.next_token();
    REQUIRE(tok6.type == TokenType::Eof);
}
