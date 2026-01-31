#include <lucid/frontend/parser.hpp>
#include <lucid/frontend/ast_printer.hpp>

// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include <catch_amalgamated.hpp>
#endif

#include <string>
#include <vector>
#include <iostream>

using namespace lucid;
using namespace lucid::ast;

// ===== Test Helpers =====

auto parse_ok(std::string_view source) -> std::unique_ptr<Program> {
    auto result = parse_source(source);
    if (!result.is_ok()) {
        // Print errors for debugging
        for (const auto& err : result.errors) {
            INFO("Parse error: " << err.message);
        }
    }
    REQUIRE(result.is_ok());
    return std::move(*result.program);
}

auto parse_error(std::string_view source) -> std::vector<ParseError> {
    auto result = parse_source(source);
    REQUIRE(result.has_errors());
    return result.errors;
}

// Helper to parse just an expression (for testing)
auto parse_expr(std::string_view source) -> std::unique_ptr<Expr> {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));

    // Manually call parse_expression
    auto expr = parser.parse_expression();
    REQUIRE(expr != nullptr);
    return expr;
}

template<typename T>
auto as(Expr* expr) -> T* {
    return dynamic_cast<T*>(expr);
}

// ===== Literal Tests =====

TEST_CASE("Parser: Integer literals", "[parser]") {
    auto expr = parse_expr("42");
    REQUIRE(expr->kind == ExprKind::IntLiteral);

    auto* int_lit = as<IntLiteralExpr>(expr.get());
    REQUIRE(int_lit != nullptr);
    REQUIRE(int_lit->value == 42);
}

TEST_CASE("Parser: Multiple integer values", "[parser]") {
    auto expr1 = parse_expr("0");
    auto* lit1 = as<IntLiteralExpr>(expr1.get());
    REQUIRE(lit1->value == 0);

    auto expr2 = parse_expr("1000");
    auto* lit2 = as<IntLiteralExpr>(expr2.get());
    REQUIRE(lit2->value == 1000);

    auto expr3 = parse_expr("1_000_000");
    auto* lit3 = as<IntLiteralExpr>(expr3.get());
    REQUIRE(lit3->value == 1000000);
}

TEST_CASE("Parser: Float literals", "[parser]") {
    auto expr = parse_expr("3.14");
    REQUIRE(expr->kind == ExprKind::FloatLiteral);

    auto* float_lit = as<FloatLiteralExpr>(expr.get());
    REQUIRE(float_lit != nullptr);
    REQUIRE(float_lit->value == 3.14);
}

TEST_CASE("Parser: Float with exponent", "[parser]") {
    auto expr1 = parse_expr("1.5e10");
    auto* lit1 = as<FloatLiteralExpr>(expr1.get());
    REQUIRE(lit1->value == 1.5e10);

    auto expr2 = parse_expr("2.5e-3");
    auto* lit2 = as<FloatLiteralExpr>(expr2.get());
    REQUIRE(lit2->value == 2.5e-3);
}

TEST_CASE("Parser: String literals", "[parser]") {
    auto expr = parse_expr("\"hello\"");
    REQUIRE(expr->kind == ExprKind::StringLiteral);

    auto* str_lit = as<StringLiteralExpr>(expr.get());
    REQUIRE(str_lit != nullptr);
    REQUIRE(str_lit->value == "hello");
}

TEST_CASE("Parser: String with escapes", "[parser]") {
    auto expr = parse_expr("\"hello\\nworld\"");
    auto* str_lit = as<StringLiteralExpr>(expr.get());
    REQUIRE(str_lit->value == "hello\nworld");
}

TEST_CASE("Parser: Boolean literals", "[parser]") {
    auto expr1 = parse_expr("true");
    REQUIRE(expr1->kind == ExprKind::BoolLiteral);
    auto* bool_lit1 = as<BoolLiteralExpr>(expr1.get());
    REQUIRE(bool_lit1->value == true);

    auto expr2 = parse_expr("false");
    REQUIRE(expr2->kind == ExprKind::BoolLiteral);
    auto* bool_lit2 = as<BoolLiteralExpr>(expr2.get());
    REQUIRE(bool_lit2->value == false);
}

TEST_CASE("Parser: Identifiers", "[parser]") {
    auto expr = parse_expr("foo");
    REQUIRE(expr->kind == ExprKind::Identifier);

    auto* ident = as<IdentifierExpr>(expr.get());
    REQUIRE(ident != nullptr);
    REQUIRE(ident->name == "foo");
}

TEST_CASE("Parser: Various identifiers", "[parser]") {
    auto expr1 = parse_expr("x");
    auto* id1 = as<IdentifierExpr>(expr1.get());
    REQUIRE(id1->name == "x");

    auto expr2 = parse_expr("bar_baz");
    auto* id2 = as<IdentifierExpr>(expr2.get());
    REQUIRE(id2->name == "bar_baz");

    auto expr3 = parse_expr("_internal");
    auto* id3 = as<IdentifierExpr>(expr3.get());
    REQUIRE(id3->name == "_internal");

    auto expr4 = parse_expr("value123");
    auto* id4 = as<IdentifierExpr>(expr4.get());
    REQUIRE(id4->name == "value123");
}

// ===== Binary Operator Tests =====

TEST_CASE("Parser: Simple binary operators", "[parser]") {
    auto expr = parse_expr("1 + 2");
    REQUIRE(expr->kind == ExprKind::Binary);

    auto* bin = as<BinaryExpr>(expr.get());
    REQUIRE(bin->op == BinaryOp::Add);
    REQUIRE(as<IntLiteralExpr>(bin->left.get())->value == 1);
    REQUIRE(as<IntLiteralExpr>(bin->right.get())->value == 2);
}

TEST_CASE("Parser: All arithmetic operators", "[parser]") {
    auto expr1 = parse_expr("10 - 5");
    REQUIRE(as<BinaryExpr>(expr1.get())->op == BinaryOp::Sub);

    auto expr2 = parse_expr("3 * 4");
    REQUIRE(as<BinaryExpr>(expr2.get())->op == BinaryOp::Mul);

    auto expr3 = parse_expr("20 / 4");
    REQUIRE(as<BinaryExpr>(expr3.get())->op == BinaryOp::Div);

    auto expr4 = parse_expr("17 % 5");
    REQUIRE(as<BinaryExpr>(expr4.get())->op == BinaryOp::Mod);

    auto expr5 = parse_expr("2 ** 3");
    REQUIRE(as<BinaryExpr>(expr5.get())->op == BinaryOp::Pow);
}

TEST_CASE("Parser: Comparison operators", "[parser]") {
    auto expr1 = parse_expr("x == y");
    REQUIRE(as<BinaryExpr>(expr1.get())->op == BinaryOp::Eq);

    auto expr2 = parse_expr("a != b");
    REQUIRE(as<BinaryExpr>(expr2.get())->op == BinaryOp::Ne);

    auto expr3 = parse_expr("x < y");
    REQUIRE(as<BinaryExpr>(expr3.get())->op == BinaryOp::Lt);

    auto expr4 = parse_expr("x > y");
    REQUIRE(as<BinaryExpr>(expr4.get())->op == BinaryOp::Gt);

    auto expr5 = parse_expr("x <= y");
    REQUIRE(as<BinaryExpr>(expr5.get())->op == BinaryOp::Le);

    auto expr6 = parse_expr("x >= y");
    REQUIRE(as<BinaryExpr>(expr6.get())->op == BinaryOp::Ge);
}

TEST_CASE("Parser: Logical operators", "[parser]") {
    auto expr1 = parse_expr("a and b");
    REQUIRE(as<BinaryExpr>(expr1.get())->op == BinaryOp::And);

    auto expr2 = parse_expr("x or y");
    REQUIRE(as<BinaryExpr>(expr2.get())->op == BinaryOp::Or);
}

// ===== Operator Precedence Tests =====

TEST_CASE("Parser: Multiplication before addition", "[parser]") {
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    auto expr = parse_expr("1 + 2 * 3");
    auto* add = as<BinaryExpr>(expr.get());
    REQUIRE(add->op == BinaryOp::Add);

    auto* left = as<IntLiteralExpr>(add->left.get());
    REQUIRE(left->value == 1);

    auto* right = as<BinaryExpr>(add->right.get());
    REQUIRE(right->op == BinaryOp::Mul);
    REQUIRE(as<IntLiteralExpr>(right->left.get())->value == 2);
    REQUIRE(as<IntLiteralExpr>(right->right.get())->value == 3);
}

TEST_CASE("Parser: Power before multiplication", "[parser]") {
    // 2 * 3 ** 4 should parse as 2 * (3 ** 4)
    auto expr = parse_expr("2 * 3 ** 4");
    auto* mul = as<BinaryExpr>(expr.get());
    REQUIRE(mul->op == BinaryOp::Mul);

    auto* right = as<BinaryExpr>(mul->right.get());
    REQUIRE(right->op == BinaryOp::Pow);
}

TEST_CASE("Parser: Comparison before logical and", "[parser]") {
    // a > b and c < d should parse as (a > b) and (c < d)
    auto expr = parse_expr("a > b and c < d");
    auto* and_expr = as<BinaryExpr>(expr.get());
    REQUIRE(and_expr->op == BinaryOp::And);

    auto* left = as<BinaryExpr>(and_expr->left.get());
    REQUIRE(left->op == BinaryOp::Gt);

    auto* right = as<BinaryExpr>(and_expr->right.get());
    REQUIRE(right->op == BinaryOp::Lt);
}

TEST_CASE("Parser: And before or", "[parser]") {
    // a or b and c should parse as a or (b and c)
    auto expr = parse_expr("a or b and c");
    auto* or_expr = as<BinaryExpr>(expr.get());
    REQUIRE(or_expr->op == BinaryOp::Or);

    auto* right = as<BinaryExpr>(or_expr->right.get());
    REQUIRE(right->op == BinaryOp::And);
}

TEST_CASE("Parser: Right associativity of power", "[parser]") {
    // 2 ** 3 ** 2 should parse as 2 ** (3 ** 2) = 2 ** 9 = 512
    auto expr = parse_expr("2 ** 3 ** 2");
    auto* pow1 = as<BinaryExpr>(expr.get());
    REQUIRE(pow1->op == BinaryOp::Pow);

    auto* left = as<IntLiteralExpr>(pow1->left.get());
    REQUIRE(left->value == 2);

    auto* right = as<BinaryExpr>(pow1->right.get());
    REQUIRE(right->op == BinaryOp::Pow);
    REQUIRE(as<IntLiteralExpr>(right->left.get())->value == 3);
    REQUIRE(as<IntLiteralExpr>(right->right.get())->value == 2);
}

TEST_CASE("Parser: Left associativity of addition", "[parser]") {
    // 1 + 2 + 3 should parse as (1 + 2) + 3
    auto expr = parse_expr("1 + 2 + 3");
    auto* add2 = as<BinaryExpr>(expr.get());
    REQUIRE(add2->op == BinaryOp::Add);

    auto* left = as<BinaryExpr>(add2->left.get());
    REQUIRE(left->op == BinaryOp::Add);
    REQUIRE(as<IntLiteralExpr>(left->left.get())->value == 1);
    REQUIRE(as<IntLiteralExpr>(left->right.get())->value == 2);

    auto* right = as<IntLiteralExpr>(add2->right.get());
    REQUIRE(right->value == 3);
}

// ===== Unary Operator Tests =====

TEST_CASE("Parser: Unary not", "[parser]") {
    auto expr = parse_expr("not x");
    auto* unary = as<UnaryExpr>(expr.get());
    REQUIRE(unary->op == UnaryOp::Not);
    REQUIRE(as<IdentifierExpr>(unary->operand.get())->name == "x");
}

TEST_CASE("Parser: Unary minus", "[parser]") {
    auto expr = parse_expr("-42");
    auto* unary = as<UnaryExpr>(expr.get());
    REQUIRE(unary->op == UnaryOp::Neg);
    REQUIRE(as<IntLiteralExpr>(unary->operand.get())->value == 42);
}

TEST_CASE("Parser: Unary plus", "[parser]") {
    auto expr = parse_expr("+3.14");
    auto* unary = as<UnaryExpr>(expr.get());
    REQUIRE(unary->op == UnaryOp::Pos);
}

TEST_CASE("Parser: Unary before binary", "[parser]") {
    // not a and b should parse as (not a) and b
    auto expr = parse_expr("not a and b");
    auto* and_expr = as<BinaryExpr>(expr.get());
    REQUIRE(and_expr->op == BinaryOp::And);

    auto* left = as<UnaryExpr>(and_expr->left.get());
    REQUIRE(left->op == UnaryOp::Not);
}

TEST_CASE("Parser: Double negative", "[parser]") {
    auto expr = parse_expr("--x");
    auto* outer = as<UnaryExpr>(expr.get());
    REQUIRE(outer->op == UnaryOp::Neg);

    auto* inner = as<UnaryExpr>(outer->operand.get());
    REQUIRE(inner->op == UnaryOp::Neg);
}

// ===== Postfix Operator Tests =====

TEST_CASE("Parser: Function call no args", "[parser]") {
    auto expr = parse_expr("foo()");
    auto* call = as<CallExpr>(expr.get());
    REQUIRE(call != nullptr);
    REQUIRE(as<IdentifierExpr>(call->callee.get())->name == "foo");
    REQUIRE(call->arguments.empty());
}

TEST_CASE("Parser: Function call with args", "[parser]") {
    auto expr = parse_expr("add(1, 2)");
    auto* call = as<CallExpr>(expr.get());
    REQUIRE(call != nullptr);
    REQUIRE(call->arguments.size() == 2);
    REQUIRE(as<IntLiteralExpr>(call->arguments[0].get())->value == 1);
    REQUIRE(as<IntLiteralExpr>(call->arguments[1].get())->value == 2);
}

TEST_CASE("Parser: Chained function calls", "[parser]") {
    auto expr = parse_expr("f(x)(y)");
    auto* outer_call = as<CallExpr>(expr.get());
    REQUIRE(outer_call != nullptr);

    auto* inner_call = as<CallExpr>(outer_call->callee.get());
    REQUIRE(inner_call != nullptr);
}

TEST_CASE("Parser: Method call", "[parser]") {
    auto expr = parse_expr("list.map(f)");
    auto* method = as<MethodCallExpr>(expr.get());
    REQUIRE(method != nullptr);
    REQUIRE(method->method_name == "map");
    REQUIRE(as<IdentifierExpr>(method->object.get())->name == "list");
    REQUIRE(method->arguments.size() == 1);
}

TEST_CASE("Parser: Index access", "[parser]") {
    auto expr = parse_expr("arr[0]");
    auto* index = as<IndexExpr>(expr.get());
    REQUIRE(index != nullptr);
    REQUIRE(as<IdentifierExpr>(index->object.get())->name == "arr");
    REQUIRE(as<IntLiteralExpr>(index->index.get())->value == 0);
}

TEST_CASE("Parser: Chained indexing", "[parser]") {
    auto expr = parse_expr("matrix[i][j]");
    auto* outer = as<IndexExpr>(expr.get());
    REQUIRE(outer != nullptr);

    auto* inner = as<IndexExpr>(outer->object.get());
    REQUIRE(inner != nullptr);
}

// ===== Complex Expression Tests =====

TEST_CASE("Parser: Complex arithmetic", "[parser]") {
    // (1 + 2) * 3 - 4 / 2
    auto expr = parse_expr("1 + 2 * 3 - 4 / 2");
    // Should parse as: ((1 + (2 * 3)) - (4 / 2))
    auto* sub = as<BinaryExpr>(expr.get());
    REQUIRE(sub->op == BinaryOp::Sub);
}

TEST_CASE("Parser: Function call in expression", "[parser]") {
    auto expr = parse_expr("x + f(y)");
    auto* add = as<BinaryExpr>(expr.get());
    REQUIRE(add->op == BinaryOp::Add);

    auto* call = as<CallExpr>(add->right.get());
    REQUIRE(call != nullptr);
}

// ===== Tuple Tests =====

TEST_CASE("Parser: Empty tuple", "[parser]") {
    auto expr = parse_expr("()");
    auto* tuple = as<TupleExpr>(expr.get());
    REQUIRE(tuple != nullptr);
    REQUIRE(tuple->elements.empty());
}

TEST_CASE("Parser: Single element tuple", "[parser]") {
    auto expr = parse_expr("(42,)");
    auto* tuple = as<TupleExpr>(expr.get());
    REQUIRE(tuple != nullptr);
    REQUIRE(tuple->elements.size() == 1);
    REQUIRE(as<IntLiteralExpr>(tuple->elements[0].get())->value == 42);
}

TEST_CASE("Parser: Two element tuple", "[parser]") {
    auto expr = parse_expr("(1, 2)");
    auto* tuple = as<TupleExpr>(expr.get());
    REQUIRE(tuple != nullptr);
    REQUIRE(tuple->elements.size() == 2);
    REQUIRE(as<IntLiteralExpr>(tuple->elements[0].get())->value == 1);
    REQUIRE(as<IntLiteralExpr>(tuple->elements[1].get())->value == 2);
}

TEST_CASE("Parser: Tuple with trailing comma", "[parser]") {
    auto expr = parse_expr("(1, 2, 3,)");
    auto* tuple = as<TupleExpr>(expr.get());
    REQUIRE(tuple != nullptr);
    REQUIRE(tuple->elements.size() == 3);
}

TEST_CASE("Parser: Grouped expression vs tuple", "[parser]") {
    // (42) is a grouped expression, not a tuple
    auto expr1 = parse_expr("(42)");
    REQUIRE(expr1->kind == ExprKind::IntLiteral);

    // (42,) is a tuple
    auto expr2 = parse_expr("(42,)");
    REQUIRE(expr2->kind == ExprKind::Tuple);
}

TEST_CASE("Parser: Nested tuples", "[parser]") {
    auto expr = parse_expr("((1, 2), (3, 4))");
    auto* outer = as<TupleExpr>(expr.get());
    REQUIRE(outer != nullptr);
    REQUIRE(outer->elements.size() == 2);

    auto* first = as<TupleExpr>(outer->elements[0].get());
    REQUIRE(first != nullptr);
    REQUIRE(first->elements.size() == 2);
}

// ===== List Tests =====

TEST_CASE("Parser: Empty list", "[parser]") {
    auto expr = parse_expr("[]");
    auto* list = as<ListExpr>(expr.get());
    REQUIRE(list != nullptr);
    REQUIRE(list->elements.empty());
}

TEST_CASE("Parser: List with elements", "[parser]") {
    auto expr = parse_expr("[1, 2, 3]");
    auto* list = as<ListExpr>(expr.get());
    REQUIRE(list != nullptr);
    REQUIRE(list->elements.size() == 3);
    REQUIRE(as<IntLiteralExpr>(list->elements[0].get())->value == 1);
    REQUIRE(as<IntLiteralExpr>(list->elements[1].get())->value == 2);
    REQUIRE(as<IntLiteralExpr>(list->elements[2].get())->value == 3);
}

TEST_CASE("Parser: List with trailing comma", "[parser]") {
    auto expr = parse_expr("[1, 2, 3,]");
    auto* list = as<ListExpr>(expr.get());
    REQUIRE(list->elements.size() == 3);
}

TEST_CASE("Parser: Nested lists", "[parser]") {
    auto expr = parse_expr("[[1, 2], [3, 4]]");
    auto* outer = as<ListExpr>(expr.get());
    REQUIRE(outer != nullptr);
    REQUIRE(outer->elements.size() == 2);

    auto* first = as<ListExpr>(outer->elements[0].get());
    REQUIRE(first != nullptr);
    REQUIRE(first->elements.size() == 2);
}

// ===== Lambda Tests =====

TEST_CASE("Parser: Lambda no params expression body", "[parser]") {
    auto expr = parse_expr("lambda: 42");
    auto* lambda = as<LambdaExpr>(expr.get());
    REQUIRE(lambda != nullptr);
    REQUIRE(lambda->parameters.empty());
    REQUIRE(lambda->body->kind == ExprKind::IntLiteral);
}

TEST_CASE("Parser: Lambda one param", "[parser]") {
    auto expr = parse_expr("lambda x: x + 1");
    auto* lambda = as<LambdaExpr>(expr.get());
    REQUIRE(lambda != nullptr);
    REQUIRE(lambda->parameters.size() == 1);
    REQUIRE(lambda->parameters[0] == "x");
    REQUIRE(lambda->body->kind == ExprKind::Binary);
}

TEST_CASE("Parser: Lambda multiple params", "[parser]") {
    auto expr = parse_expr("lambda x, y: x + y");
    auto* lambda = as<LambdaExpr>(expr.get());
    REQUIRE(lambda != nullptr);
    REQUIRE(lambda->parameters.size() == 2);
    REQUIRE(lambda->parameters[0] == "x");
    REQUIRE(lambda->parameters[1] == "y");
}

TEST_CASE("Parser: Lambda with block body", "[parser]") {
    auto expr = parse_expr("lambda x: { return x + 1 }");
    auto* lambda = as<LambdaExpr>(expr.get());
    REQUIRE(lambda != nullptr);
    REQUIRE(lambda->parameters.size() == 1);
    REQUIRE(lambda->body->kind == ExprKind::Block);

    auto* block = as<BlockExpr>(lambda->body.get());
    REQUIRE(block->statements.size() == 1);
    REQUIRE(block->statements[0]->kind == StmtKind::Return);
}

TEST_CASE("Parser: Nested lambdas", "[parser]") {
    auto expr = parse_expr("lambda x: lambda y: x + y");
    auto* outer = as<LambdaExpr>(expr.get());
    REQUIRE(outer != nullptr);

    auto* inner = as<LambdaExpr>(outer->body.get());
    REQUIRE(inner != nullptr);
}

// ===== If Expression Tests =====

TEST_CASE("Parser: If without else", "[parser]") {
    auto expr = parse_expr("if x > 0 { return x }");
    auto* if_expr = as<IfExpr>(expr.get());
    REQUIRE(if_expr != nullptr);
    REQUIRE(if_expr->condition->kind == ExprKind::Binary);
    REQUIRE(if_expr->then_branch->kind == ExprKind::Block);
    REQUIRE(!if_expr->else_branch.has_value());
}

TEST_CASE("Parser: If with else", "[parser]") {
    auto expr = parse_expr("if x > 0 { return x } else { return 0 }");
    auto* if_expr = as<IfExpr>(expr.get());
    REQUIRE(if_expr != nullptr);
    REQUIRE(if_expr->then_branch->kind == ExprKind::Block);
    REQUIRE(if_expr->else_branch.has_value());
    REQUIRE((*if_expr->else_branch)->kind == ExprKind::Block);
}

TEST_CASE("Parser: If else if", "[parser]") {
    auto expr = parse_expr("if x > 0 { return 1 } else if x < 0 { return -1 } else { return 0 }");
    auto* if_expr = as<IfExpr>(expr.get());
    REQUIRE(if_expr != nullptr);
    REQUIRE(if_expr->else_branch.has_value());

    // else branch should be another if expression
    auto* else_if = as<IfExpr>((*if_expr->else_branch).get());
    REQUIRE(else_if != nullptr);
    REQUIRE(else_if->else_branch.has_value());
}

// ===== Block Expression Tests =====

TEST_CASE("Parser: Block with return", "[parser]") {
    auto expr = parse_expr("{ return 42 }");
    auto* block = as<BlockExpr>(expr.get());
    REQUIRE(block != nullptr);
    REQUIRE(block->statements.size() == 1);
    REQUIRE(block->statements[0]->kind == StmtKind::Return);
}

TEST_CASE("Parser: Block with let and return", "[parser]") {
    auto expr = parse_expr("{ let x = 10 return x }");
    auto* block = as<BlockExpr>(expr.get());
    REQUIRE(block != nullptr);
    REQUIRE(block->statements.size() == 2);
    REQUIRE(block->statements[0]->kind == StmtKind::Let);
    REQUIRE(block->statements[1]->kind == StmtKind::Return);
}

// ===== Complex Expression Combinations =====

TEST_CASE("Parser: Lambda in list", "[parser]") {
    auto expr = parse_expr("[lambda x: x + 1, lambda y: y * 2]");
    auto* list = as<ListExpr>(expr.get());
    REQUIRE(list != nullptr);
    REQUIRE(list->elements.size() == 2);
    REQUIRE(list->elements[0]->kind == ExprKind::Lambda);
    REQUIRE(list->elements[1]->kind == ExprKind::Lambda);
}

TEST_CASE("Parser: Function call with lambda", "[parser]") {
    auto expr = parse_expr("map(lambda x: x * 2, list)");
    auto* call = as<CallExpr>(expr.get());
    REQUIRE(call != nullptr);
    REQUIRE(call->arguments.size() == 2);
    REQUIRE(call->arguments[0]->kind == ExprKind::Lambda);
}

TEST_CASE("Parser: Tuple of expressions", "[parser]") {
    auto expr = parse_expr("(x + 1, y * 2, f())");
    auto* tuple = as<TupleExpr>(expr.get());
    REQUIRE(tuple != nullptr);
    REQUIRE(tuple->elements.size() == 3);
    REQUIRE(tuple->elements[0]->kind == ExprKind::Binary);
    REQUIRE(tuple->elements[1]->kind == ExprKind::Binary);
    REQUIRE(tuple->elements[2]->kind == ExprKind::Call);
}

// ===== Error Tests =====

TEST_CASE("Parser: Error on unexpected token", "[parser]") {
    auto errors = parse_error("@");
    REQUIRE(!errors.empty());
}

// ===== AST Printer Tests (Day 6) =====

TEST_CASE("AST Printer: Simple expression", "[parser][printer]") {
    auto expr = parse_expr("42");
    ASTPrinter printer;
    auto output = printer.print_expr(expr.get());
    REQUIRE(output.find("IntLiteral(42)") != std::string::npos);
}

// ===== Integration Tests (Day 6) =====

TEST_CASE("Integration: Multi-function program", "[parser][integration]") {
    auto prog = parse_ok(R"(
        function helper(n: Int) returns Int {
            return n + 1
        }

        function main() returns Int {
            return helper(41)
        }
    )");

    REQUIRE(prog->functions.size() == 2);
    REQUIRE(prog->functions[0]->name == "helper");
    REQUIRE(prog->functions[1]->name == "main");
}

// ===== Error Recovery Tests (Day 6) =====

TEST_CASE("Error Recovery: Missing closing brace", "[parser][error]") {
    auto errors = parse_error(R"(
function test() returns Int {
    let x = 42
)");
    REQUIRE(!errors.empty());
}
