#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include <catch_amalgamated.hpp>
#endif

#include <lucid/semantic/type_checker.hpp>
#include <lucid/frontend/parser.hpp>
#include <lucid/frontend/lexer.hpp>
#include <fmt/format.h>
#include <stdexcept>

using namespace lucid;
using namespace lucid::semantic;

// Helper function to type check an expression within a minimal program
auto type_check_expr(const std::string& expr_str) -> std::pair<std::unique_ptr<SemanticType>, TypeCheckResult> {
    // Wrap expression in a function for parsing
    std::string program = fmt::format(R"(
        function test() returns Int {{
            return {}
        }}
    )", expr_str);

    Lexer lexer(program);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto parse_result = parser.parse();

    if (!parse_result.is_ok() || !parse_result.program.has_value()) {
        throw std::runtime_error("Failed to parse test expression");
    }

    auto& ast = *parse_result.program.value();
    TypeChecker checker;
    auto result = checker.check_program(ast);

    // Extract the return expression from the function
    auto& func = ast.functions[0];
    if (func->body->kind != ast::ExprKind::Block) {
        throw std::runtime_error("Expected block expression");
    }

    auto& block = static_cast<ast::BlockExpr&>(*func->body);
    if (block.statements.empty()) {
        throw std::runtime_error("Expected at least one statement");
    }

    auto& last_stmt = block.statements.back();
    if (last_stmt->kind != ast::StmtKind::Return) {
        throw std::runtime_error("Expected return statement");
    }

    auto& return_stmt = static_cast<ast::ReturnStmt&>(*last_stmt);
    if (!return_stmt.value) {
        throw std::runtime_error("Expected return value");
    }

    // Type check the return expression
    auto expr_type = checker.check_expression(*return_stmt.value);

    return {std::move(expr_type), std::move(result)};
}

// ===== Literal Type Inference Tests =====

TEST_CASE("Type checking: Int literal", "[type_checker][literals]") {
    auto [type, result] = type_check_expr("42");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Float literal", "[type_checker][literals]") {
    auto [type, result] = type_check_expr("3.14");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: String literal", "[type_checker][literals]") {
    auto [type, result] = type_check_expr(R"("hello")");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::String);
}

TEST_CASE("Type checking: Bool literal true", "[type_checker][literals]") {
    auto [type, result] = type_check_expr("true");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Bool literal false", "[type_checker][literals]") {
    auto [type, result] = type_check_expr("false");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

// ===== Binary Arithmetic Operator Tests =====

TEST_CASE("Type checking: Int + Int = Int", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("1 + 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Float + Float = Float", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("1.5 + 2.5");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Int + Float = Float (promotion)", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("1 + 2.5");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Float + Int = Float (promotion)", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("1.5 + 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Int - Int = Int", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("10 - 3");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Int * Int = Int", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("3 * 4");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Int / Int = Int", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("10 / 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Int % Int = Int", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("10 % 3");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Int ** Int = Int (power)", "[type_checker][binary][arithmetic]") {
    auto [type, result] = type_check_expr("2 ** 8");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

// ===== Binary Comparison Operator Tests =====

TEST_CASE("Type checking: Int == Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("1 == 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Int != Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("1 != 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Int < Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("1 < 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Int > Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("2 > 1");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Int <= Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("1 <= 2");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Int >= Int = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("2 >= 1");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Float < Float = Bool", "[type_checker][binary][comparison]") {
    auto [type, result] = type_check_expr("1.5 < 2.5");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

// ===== Binary Logical Operator Tests =====

TEST_CASE("Type checking: Bool and Bool = Bool", "[type_checker][binary][logical]") {
    auto [type, result] = type_check_expr("true and false");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Bool or Bool = Bool", "[type_checker][binary][logical]") {
    auto [type, result] = type_check_expr("true or false");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

// ===== Unary Operator Tests =====

TEST_CASE("Type checking: -Int = Int", "[type_checker][unary]") {
    auto [type, result] = type_check_expr("-42");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: +Float = Float", "[type_checker][unary]") {
    auto [type, result] = type_check_expr("+3.14");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: not Bool = Bool", "[type_checker][unary]") {
    auto [type, result] = type_check_expr("not true");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

// ===== Tuple Type Inference Tests =====

TEST_CASE("Type checking: Tuple (Int, Float)", "[type_checker][tuple]") {
    auto [type, result] = type_check_expr("(1, 2.5)");

    REQUIRE(type->kind == TypeKind::Tuple);
    auto* tuple = static_cast<TupleType*>(type.get());
    REQUIRE(tuple->element_types.size() == 2);

    REQUIRE(tuple->element_types[0]->kind == TypeKind::Primitive);
    auto* elem0 = static_cast<PrimitiveType*>(tuple->element_types[0].get());
    REQUIRE(elem0->primitive_kind == PrimitiveKind::Int);

    REQUIRE(tuple->element_types[1]->kind == TypeKind::Primitive);
    auto* elem1 = static_cast<PrimitiveType*>(tuple->element_types[1].get());
    REQUIRE(elem1->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Tuple (String, Bool, Int)", "[type_checker][tuple]") {
    auto [type, result] = type_check_expr(R"(("hello", true, 42))");

    REQUIRE(type->kind == TypeKind::Tuple);
    auto* tuple = static_cast<TupleType*>(type.get());
    REQUIRE(tuple->element_types.size() == 3);

    auto* elem0 = static_cast<PrimitiveType*>(tuple->element_types[0].get());
    REQUIRE(elem0->primitive_kind == PrimitiveKind::String);

    auto* elem1 = static_cast<PrimitiveType*>(tuple->element_types[1].get());
    REQUIRE(elem1->primitive_kind == PrimitiveKind::Bool);

    auto* elem2 = static_cast<PrimitiveType*>(tuple->element_types[2].get());
    REQUIRE(elem2->primitive_kind == PrimitiveKind::Int);
}

// ===== List Type Inference Tests =====

TEST_CASE("Type checking: List[Int]", "[type_checker][list]") {
    auto [type, result] = type_check_expr("[1, 2, 3]");

    REQUIRE(type->kind == TypeKind::List);
    auto* list = static_cast<ListType*>(type.get());

    REQUIRE(list->element_type->kind == TypeKind::Primitive);
    auto* elem = static_cast<PrimitiveType*>(list->element_type.get());
    REQUIRE(elem->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: List[Float]", "[type_checker][list]") {
    auto [type, result] = type_check_expr("[1.5, 2.5, 3.5]");

    REQUIRE(type->kind == TypeKind::List);
    auto* list = static_cast<ListType*>(type.get());

    REQUIRE(list->element_type->kind == TypeKind::Primitive);
    auto* elem = static_cast<PrimitiveType*>(list->element_type.get());
    REQUIRE(elem->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Empty list = List[Unknown]", "[type_checker][list]") {
    auto [type, result] = type_check_expr("[]");

    REQUIRE(type->kind == TypeKind::List);
    auto* list = static_cast<ListType*>(type.get());
    REQUIRE(list->element_type->kind == TypeKind::Unknown);
}

// ===== Type Error Detection Tests =====

TEST_CASE("Type checking: String + Int error", "[type_checker][errors]") {
    auto [type, result] = type_check_expr(R"("hello" + 5)");

    // Should still return a type (Unknown for error recovery)
    REQUIRE(result.has_errors());
    REQUIRE(result.errors.size() >= 1);
}

TEST_CASE("Type checking: not Int error", "[type_checker][errors]") {
    auto [type, result] = type_check_expr("not 42");

    REQUIRE(result.has_errors());
    REQUIRE(result.errors.size() >= 1);
}

TEST_CASE("Type checking: Bool + Bool error", "[type_checker][errors]") {
    auto [type, result] = type_check_expr("true + false");

    REQUIRE(result.has_errors());
    REQUIRE(result.errors.size() >= 1);
}

TEST_CASE("Type checking: Int and Int error", "[type_checker][errors]") {
    auto [type, result] = type_check_expr("1 and 2");

    REQUIRE(result.has_errors());
    REQUIRE(result.errors.size() >= 1);
}

TEST_CASE("Type checking: Heterogeneous list error", "[type_checker][errors]") {
    auto [type, result] = type_check_expr("[1, 2.5, 3]");

    REQUIRE(result.has_errors());
    // Should have at least one error for type mismatch
    REQUIRE(result.errors.size() >= 1);
}

// ===== Complex Expression Tests =====

TEST_CASE("Type checking: Complex arithmetic (1 + 2) * 3", "[type_checker][complex]") {
    auto [type, result] = type_check_expr("(1 + 2) * 3");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Int);
}

TEST_CASE("Type checking: Mixed arithmetic with promotion (1 + 2.5) * 3", "[type_checker][complex]") {
    auto [type, result] = type_check_expr("(1 + 2.5) * 3");

    // (1 + 2.5) = Float, Float * 3 = Float
    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Float);
}

TEST_CASE("Type checking: Comparison chain 1 < 2 and 3 > 1", "[type_checker][complex]") {
    auto [type, result] = type_check_expr("1 < 2 and 3 > 1");

    REQUIRE(type->kind == TypeKind::Primitive);
    auto* prim = static_cast<PrimitiveType*>(type.get());
    REQUIRE(prim->primitive_kind == PrimitiveKind::Bool);
}

TEST_CASE("Type checking: Nested tuple ((1, 2), (3, 4))", "[type_checker][complex]") {
    auto [type, result] = type_check_expr("((1, 2), (3, 4))");

    REQUIRE(type->kind == TypeKind::Tuple);
    auto* tuple = static_cast<TupleType*>(type.get());
    REQUIRE(tuple->element_types.size() == 2);

    // First element is a tuple
    REQUIRE(tuple->element_types[0]->kind == TypeKind::Tuple);
    auto* inner0 = static_cast<TupleType*>(tuple->element_types[0].get());
    REQUIRE(inner0->element_types.size() == 2);

    // Second element is a tuple
    REQUIRE(tuple->element_types[1]->kind == TypeKind::Tuple);
    auto* inner1 = static_cast<TupleType*>(tuple->element_types[1].get());
    REQUIRE(inner1->element_types.size() == 2);
}

TEST_CASE("Type checking: List of tuples [(1, 2), (3, 4)]", "[type_checker][complex]") {
    auto [type, result] = type_check_expr("[(1, 2), (3, 4)]");

    REQUIRE(type->kind == TypeKind::List);
    auto* list = static_cast<ListType*>(type.get());

    REQUIRE(list->element_type->kind == TypeKind::Tuple);
    auto* tuple = static_cast<TupleType*>(list->element_type.get());
    REQUIRE(tuple->element_types.size() == 2);
}
