// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include "catch_amalgamated.hpp"
#endif

#include <lucid/backend/vm.hpp>
#include <lucid/backend/bytecode.hpp>
#include <lucid/backend/compiler.hpp>
#include <lucid/frontend/parser.hpp>
#include <lucid/frontend/lexer.hpp>
#include <lucid/semantic/type_checker.hpp>
#include <fmt/format.h>

using namespace lucid::backend;
using namespace lucid;

// Helper to compile a simple expression to bytecode
auto compile_expression(const std::string& expr_source, const std::string& return_type = "Int") -> Bytecode {
    // Wrap expression in a function for testing
    std::string source = fmt::format(R"(
        function test() returns {} {{
            return {}
        }}
    )", return_type, expr_source);

    Lexer lexer(source, "test");
    Parser parser(lexer.tokenize());
    auto parse_result = parser.parse();

    if (!parse_result.is_ok()) {
        throw std::runtime_error("Parse error");
    }

    semantic::TypeChecker checker;
    auto type_result = checker.check_program(*parse_result.program.value());
    if (!type_result.errors.empty()) {
        throw std::runtime_error("Type check error");
    }

    Compiler compiler;
    return compiler.compile(parse_result.program.value().get());
}

// Helper to execute a simple expression and get result
auto execute_expression(const std::string& expr, const std::string& return_type = "Int") -> Value {
    auto bytecode = compile_expression(expr, return_type);
    VM vm;
    return vm.call_function(bytecode, "test", {});
}

// ===== Day 1: Basic Execution Tests =====

TEST_CASE("VM: Execute integer literal", "[vm][day1]") {
    auto result = execute_expression("42");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Execute boolean literals", "[vm][day1]") {
    SECTION("True") {
        auto result = execute_expression("true", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("False") {
        auto result = execute_expression("false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Arithmetic - Addition", "[vm][day1][arithmetic]") {
    SECTION("Int + Int") {
        auto result = execute_expression("3 + 4");
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 7);
    }

    SECTION("Large numbers") {
        auto result = execute_expression("1000 + 2000");
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 3000);
    }
}

TEST_CASE("VM: Arithmetic - Subtraction", "[vm][day1][arithmetic]") {
    auto result = execute_expression("10 - 3");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 7);
}

TEST_CASE("VM: Arithmetic - Multiplication", "[vm][day1][arithmetic]") {
    auto result = execute_expression("6 * 7");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Arithmetic - Division", "[vm][day1][arithmetic]") {
    SECTION("Normal division") {
        auto result = execute_expression("20 / 4");
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 5);
    }

    SECTION("Division by zero") {
        REQUIRE_THROWS_AS(execute_expression("10 / 0"), std::runtime_error);
    }
}

TEST_CASE("VM: Arithmetic - Modulo", "[vm][day1][arithmetic]") {
    auto result = execute_expression("10 % 3");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 1);
}

TEST_CASE("VM: Arithmetic - Power", "[vm][day1][arithmetic]") {
    auto result = execute_expression("2 ** 8");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 256);
}

TEST_CASE("VM: Arithmetic - Complex expression", "[vm][day1][arithmetic]") {
    // 2 + 3 * 4 = 2 + 12 = 14
    auto result = execute_expression("2 + 3 * 4");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 14);
}

TEST_CASE("VM: Comparison - Equality", "[vm][day1][comparison]") {
    SECTION("Equal integers") {
        auto result = execute_expression("5 == 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("Unequal integers") {
        auto result = execute_expression("5 == 3", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Comparison - Not equal", "[vm][day1][comparison]") {
    SECTION("Different values") {
        auto result = execute_expression("5 != 3", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("Same values") {
        auto result = execute_expression("5 != 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Comparison - Less than", "[vm][day1][comparison]") {
    SECTION("True case") {
        auto result = execute_expression("3 < 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("False case") {
        auto result = execute_expression("5 < 3", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Comparison - Greater than", "[vm][day1][comparison]") {
    auto result = execute_expression("10 > 5", "Bool");
    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);
}

TEST_CASE("VM: Comparison - Less or equal", "[vm][day1][comparison]") {
    SECTION("Less") {
        auto result = execute_expression("3 <= 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("Equal") {
        auto result = execute_expression("5 <= 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("Greater") {
        auto result = execute_expression("7 <= 5", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Comparison - Greater or equal", "[vm][day1][comparison]") {
    auto result = execute_expression("10 >= 5", "Bool");
    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);
}

TEST_CASE("VM: Logical - AND", "[vm][day1][logical]") {
    SECTION("true and true") {
        auto result = execute_expression("true and true", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("true and false") {
        auto result = execute_expression("true and false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }

    SECTION("false and false") {
        auto result = execute_expression("false and false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Logical - OR", "[vm][day1][logical]") {
    SECTION("true or false") {
        auto result = execute_expression("true or false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("false or false") {
        auto result = execute_expression("false or false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: Logical - NOT", "[vm][day1][logical]") {
    SECTION("not true") {
        auto result = execute_expression("not true", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }

    SECTION("not false") {
        auto result = execute_expression("not false", "Bool");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }
}

TEST_CASE("VM: Unary - Negation", "[vm][day1][unary]") {
    SECTION("Negate positive") {
        auto result = execute_expression("-42");
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == -42);
    }

    SECTION("Negate negative") {
        auto result = execute_expression("-(0 - 5)");  // -(-5) = 5
        REQUIRE(result.is_int());
        REQUIRE(result.as_int() == 5);
    }
}

TEST_CASE("VM: Unary - Positive", "[vm][day1][unary]") {
    auto result = execute_expression("+42");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Complex boolean expression", "[vm][day1]") {
    // (5 > 3) and (10 < 20)
    auto result = execute_expression("(5 > 3) and (10 < 20)", "Bool");
    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);
}

// ===== Day 2: Function Call Tests =====

// Helper to compile and execute a full program
auto execute_program(const std::string& source, const std::string& entry_func = "main") -> Value {
    Lexer lexer(source, "test");
    Parser parser(lexer.tokenize());
    auto parse_result = parser.parse();

    if (!parse_result.is_ok()) {
        throw std::runtime_error("Parse error");
    }

    semantic::TypeChecker checker;
    auto type_result = checker.check_program(*parse_result.program.value());
    if (!type_result.errors.empty()) {
        std::string errors;
        for (const auto& err : type_result.errors) {
            errors += err.message + "\n";
        }
        throw std::runtime_error("Type check error: " + errors);
    }

    Compiler compiler;
    auto bytecode = compiler.compile(parse_result.program.value().get());

    VM vm;
    return vm.call_function(bytecode, entry_func, {});
}

TEST_CASE("VM: Simple function call", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function add(x: Int, y: Int) returns Int {
            return x + y
        }

        function main() returns Int {
            return add(5, 3)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 8);
}

TEST_CASE("VM: Function with no parameters", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function get_answer() returns Int {
            return 42
        }

        function main() returns Int {
            return get_answer()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Nested function calls", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function double(x: Int) returns Int {
            return x * 2
        }

        function triple(x: Int) returns Int {
            return x * 3
        }

        function main() returns Int {
            return double(triple(5))
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 30);  // triple(5)=15, double(15)=30
}

TEST_CASE("VM: Recursive function - factorial", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function factorial(n: Int) returns Int {
            return if n <= 1 {
                1
            } else {
                n * factorial(n - 1)
            }
        }

        function main() returns Int {
            return factorial(5)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 120);  // 5! = 120
}

TEST_CASE("VM: Recursive function - fibonacci", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function fib(n: Int) returns Int {
            return if n <= 1 {
                n
            } else {
                fib(n - 1) + fib(n - 2)
            }
        }

        function main() returns Int {
            return fib(10)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 55);  // fib(10) = 55
}

TEST_CASE("VM: Multiple function calls in expression", "[vm][day2][functions]") {
    auto result = execute_program(R"(
        function square(x: Int) returns Int {
            return x * x
        }

        function main() returns Int {
            return square(3) + square(4)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 25);  // 9 + 16 = 25
}

// ===== Day 3: Control Flow Tests =====

TEST_CASE("VM: If expression - true branch", "[vm][day3][control]") {
    auto result = execute_expression("if 5 > 3 { 100 } else { 200 }");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 100);
}

TEST_CASE("VM: If expression - false branch", "[vm][day3][control]") {
    auto result = execute_expression("if 3 > 5 { 100 } else { 200 }");
    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 200);
}

TEST_CASE("VM: Nested if expressions", "[vm][day3][control]") {
    auto result = execute_program(R"(
        function classify(n: Int) returns Int {
            return if n < 0 {
                0 - 1
            } else {
                if n == 0 {
                    0
                } else {
                    1
                }
            }
        }

        function main() returns Int {
            return classify(5) + classify(0) * 10 + classify(0 - 3) * 100
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 1 + 0 + (-100));  // 1 + 0 - 100 = -99
}

TEST_CASE("VM: If with comparison operators", "[vm][day3][control]") {
    SECTION("Less than") {
        auto result = execute_expression("if 3 < 5 { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }

    SECTION("Greater than") {
        auto result = execute_expression("if 10 > 5 { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }

    SECTION("Equal") {
        auto result = execute_expression("if 5 == 5 { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }

    SECTION("Not equal") {
        auto result = execute_expression("if 5 != 3 { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }
}

TEST_CASE("VM: If with logical operators", "[vm][day3][control]") {
    SECTION("AND - both true") {
        auto result = execute_expression("if (5 > 3) and (10 > 5) { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }

    SECTION("AND - one false") {
        auto result = execute_expression("if (5 > 3) and (10 < 5) { 1 } else { 0 }");
        REQUIRE(result.as_int() == 0);
    }

    SECTION("OR - one true") {
        auto result = execute_expression("if (5 < 3) or (10 > 5) { 1 } else { 0 }");
        REQUIRE(result.as_int() == 1);
    }

    SECTION("OR - both false") {
        auto result = execute_expression("if (5 < 3) or (10 < 5) { 1 } else { 0 }");
        REQUIRE(result.as_int() == 0);
    }
}

// ===== Day 4: Collections Tests =====

TEST_CASE("VM: List construction", "[vm][day4][collections][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            return nums[0]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 1);
}

TEST_CASE("VM: List indexing", "[vm][day4][collections][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [10, 20, 30, 40, 50]
            return nums[2]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 30);
}

TEST_CASE("VM: List - sum elements via indexing", "[vm][day4][collections][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3, 4, 5]
            return nums[0] + nums[1] + nums[2] + nums[3] + nums[4]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 15);
}

TEST_CASE("VM: Tuple construction", "[vm][day4][collections][tuple]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let pair = (100, 200)
            return pair[0]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 100);
}

TEST_CASE("VM: Tuple indexing", "[vm][day4][collections][tuple]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let triple = (1, 2, 3)
            return triple[0] + triple[1] + triple[2]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 6);
}

TEST_CASE("VM: Index out of bounds - list", "[vm][day4][collections][error]") {
    REQUIRE_THROWS_AS(execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            return nums[10]
        }
    )"), std::runtime_error);
}

TEST_CASE("VM: Index out of bounds - tuple", "[vm][day4][collections][error]") {
    REQUIRE_THROWS_AS(execute_program(R"(
        function main() returns Int {
            let pair = (1, 2)
            return pair[5]
        }
    )"), std::runtime_error);
}

// ===== Day 5: Let Bindings Tests =====

TEST_CASE("VM: Simple let binding", "[vm][day5][let]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let x = 42
            return x
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Multiple let bindings", "[vm][day5][let]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let x = 10
            let y = 20
            let z = 30
            return x + y + z
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 60);
}

TEST_CASE("VM: Let binding with expression", "[vm][day5][let]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let x = 5
            let y = x * 2
            let z = y + 3
            return z
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 13);  // (5 * 2) + 3 = 13
}

TEST_CASE("VM: Let binding with function call", "[vm][day5][let]") {
    auto result = execute_program(R"(
        function double(n: Int) returns Int {
            return n * 2
        }

        function main() returns Int {
            let x = double(5)
            let y = double(x)
            return y
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 20);  // double(5)=10, double(10)=20
}

TEST_CASE("VM: Let binding with if expression", "[vm][day5][let]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let x = 10
            let y = if x > 5 { x * 2 } else { x }
            return y
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 20);
}

// ===== Day 6: Built-in Methods Tests =====

TEST_CASE("VM: List.length()", "[vm][day6][methods][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3, 4, 5]
            return nums.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 5);
}

TEST_CASE("VM: List.length() - empty list", "[vm][day6][methods][list]") {
    // Test empty list by consuming all elements
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1]
            let empty = nums.tail()
            return empty.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 0);
}

TEST_CASE("VM: List.append()", "[vm][day6][methods][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            let extended = nums.append(4)
            return extended.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 4);
}

TEST_CASE("VM: List.append() - value preserved", "[vm][day6][methods][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [10, 20]
            let extended = nums.append(30)
            return extended[2]
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 30);
}

TEST_CASE("VM: List.head()", "[vm][day6][methods][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [42, 2, 3]
            return nums.head()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: List.tail()", "[vm][day6][methods][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3, 4]
            let rest = nums.tail()
            return rest.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 3);
}

TEST_CASE("VM: List.is_empty()", "[vm][day6][methods][list]") {
    SECTION("Non-empty list") {
        auto result = execute_program(R"(
            function main() returns Bool {
                let nums = [1, 2, 3]
                return nums.is_empty()
            }
        )");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }

    SECTION("Empty list") {
        // Test empty list by consuming all elements
        auto result = execute_program(R"(
            function main() returns Bool {
                let nums = [1]
                let empty = nums.tail()
                return empty.is_empty()
            }
        )");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }
}

TEST_CASE("VM: Tuple.length()", "[vm][day6][methods][tuple]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let pair = (1, 2, 3)
            return pair.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 3);
}

TEST_CASE("VM: String.length()", "[vm][day6][methods][string]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let s = "hello"
            return s.length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 5);
}

TEST_CASE("VM: String.is_empty()", "[vm][day6][methods][string]") {
    SECTION("Non-empty string") {
        auto result = execute_program(R"(
            function main() returns Bool {
                let s = "hello"
                return s.is_empty()
            }
        )");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }

    SECTION("Empty string") {
        auto result = execute_program(R"(
            function main() returns Bool {
                let s = ""
                return s.is_empty()
            }
        )");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }
}

TEST_CASE("VM: Method chaining", "[vm][day6][methods]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            return nums.append(4).append(5).length()
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 5);
}

// ===== Integration Tests =====

TEST_CASE("VM: Complete fibonacci with recursion", "[vm][integration]") {
    auto result = execute_program(R"(
        function fib(n: Int) returns Int {
            return if n <= 1 {
                n
            } else {
                fib(n - 1) + fib(n - 2)
            }
        }

        function main() returns Int {
            return fib(15)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 610);  // fib(15) = 610
}

TEST_CASE("VM: Sum list elements", "[vm][integration]") {
    auto result = execute_program(R"(
        function sum_first_n(n: Int, nums: List[Int], acc: Int) returns Int {
            return if n <= 0 {
                acc
            } else {
                sum_first_n(n - 1, nums.tail(), acc + nums.head())
            }
        }

        function main() returns Int {
            let nums = [1, 2, 3, 4, 5]
            return sum_first_n(5, nums, 0)
        }
    )");

    REQUIRE(result.is_int());
    REQUIRE(result.as_int() == 15);  // 1+2+3+4+5 = 15
}

// ===== Phase 6: Print/Println Tests =====

// Helper to execute program and capture output
auto execute_with_output(const std::string& source) -> std::pair<Value, std::string> {
    Lexer lexer(source, "test");
    Parser parser(lexer.tokenize());
    auto parse_result = parser.parse();

    if (!parse_result.is_ok()) {
        throw std::runtime_error("Parse error");
    }

    semantic::TypeChecker checker;
    auto type_result = checker.check_program(*parse_result.program.value());
    if (!type_result.errors.empty()) {
        std::string errors;
        for (const auto& err : type_result.errors) {
            errors += err.message + "\n";
        }
        throw std::runtime_error("Type check error: " + errors);
    }

    Compiler compiler;
    auto bytecode = compiler.compile(parse_result.program.value().get());

    VM vm;
    vm.use_output_buffer();  // Capture output
    auto result = vm.call_function(bytecode, "main", {});
    return {std::move(result), vm.get_output()};
}

TEST_CASE("VM: println - integer", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println(42)
            return 0
        }
    )");

    REQUIRE(output == "42\n");
}

TEST_CASE("VM: println - string", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println("Hello, World!")
            return 0
        }
    )");

    REQUIRE(output == "Hello, World!\n");
}

TEST_CASE("VM: println - bool", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println(true)
            println(false)
            return 0
        }
    )");

    REQUIRE(output == "true\nfalse\n");
}

TEST_CASE("VM: println - list", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println([1, 2, 3])
            return 0
        }
    )");

    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE("VM: println - tuple", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println((1, 2, 3))
            return 0
        }
    )");

    REQUIRE(output == "(1, 2, 3)\n");
}

TEST_CASE("VM: print - no newline", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            print("Hello")
            print(" ")
            print("World")
            return 0
        }
    )");

    REQUIRE(output == "Hello World");
}

TEST_CASE("VM: to_string - integer", "[vm][phase6][stdlib]") {
    auto result = execute_program(R"(
        function main() returns String {
            return to_string(42)
        }
    )", "main");

    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "42");
}

TEST_CASE("VM: to_string - bool", "[vm][phase6][stdlib]") {
    auto result = execute_program(R"(
        function main() returns String {
            return to_string(true)
        }
    )", "main");

    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "true");
}

TEST_CASE("VM: to_string - list", "[vm][phase6][stdlib]") {
    auto result = execute_program(R"(
        function main() returns String {
            return to_string([1, 2, 3])
        }
    )", "main");

    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "[1, 2, 3]");
}

TEST_CASE("VM: Multiple println statements", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println(1)
            println(2)
            println(3)
            return 0
        }
    )");

    REQUIRE(output == "1\n2\n3\n");
}

TEST_CASE("VM: println with expression", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function main() returns Int {
            println(2 + 3 * 4)
            return 0
        }
    )");

    REQUIRE(output == "14\n");
}

TEST_CASE("VM: println with function result", "[vm][phase6][print]") {
    auto [result, output] = execute_with_output(R"(
        function square(x: Int) returns Int {
            return x * x
        }

        function main() returns Int {
            println(square(5))
            return 0
        }
    )");

    REQUIRE(output == "25\n");
}

// ===== String Methods Tests =====

TEST_CASE("VM: String.contains", "[vm][phase6][string]") {
    SECTION("Found") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".contains("world")
            }
        )", "main");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == true);
    }

    SECTION("Not found") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".contains("xyz")
            }
        )", "main");
        REQUIRE(result.is_bool());
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: String.starts_with", "[vm][phase6][string]") {
    SECTION("Matches") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".starts_with("hello")
            }
        )", "main");
        REQUIRE(result.as_bool() == true);
    }

    SECTION("No match") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".starts_with("world")
            }
        )", "main");
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: String.ends_with", "[vm][phase6][string]") {
    SECTION("Matches") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".ends_with("world")
            }
        )", "main");
        REQUIRE(result.as_bool() == true);
    }

    SECTION("No match") {
        auto result = execute_program(R"(
            function main() returns Bool {
                return "hello world".ends_with("hello")
            }
        )", "main");
        REQUIRE(result.as_bool() == false);
    }
}

TEST_CASE("VM: String.to_upper", "[vm][phase6][string]") {
    auto result = execute_program(R"(
        function main() returns String {
            return "Hello World".to_upper()
        }
    )", "main");
    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "HELLO WORLD");
}

TEST_CASE("VM: String.to_lower", "[vm][phase6][string]") {
    auto result = execute_program(R"(
        function main() returns String {
            return "Hello World".to_lower()
        }
    )", "main");
    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "hello world");
}

TEST_CASE("VM: String.trim", "[vm][phase6][string]") {
    SECTION("Leading and trailing whitespace") {
        auto result = execute_program(R"(
            function main() returns String {
                return "  hello  ".trim()
            }
        )", "main");
        REQUIRE(result.as_string() == "hello");
    }

    SECTION("Only leading whitespace") {
        auto result = execute_program(R"(
            function main() returns String {
                return "   world".trim()
            }
        )", "main");
        REQUIRE(result.as_string() == "world");
    }

    SECTION("No whitespace") {
        auto result = execute_program(R"(
            function main() returns String {
                return "test".trim()
            }
        )", "main");
        REQUIRE(result.as_string() == "test");
    }
}

// ===== List Methods Tests =====

TEST_CASE("VM: List.reverse", "[vm][phase6][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3, 4, 5]
            let rev = nums.reverse()
            return rev[0]
        }
    )", "main");
    REQUIRE(result.as_int() == 5);
}

TEST_CASE("VM: List.reverse - full list", "[vm][phase6][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            let rev = nums.reverse()
            return rev[0] * 100 + rev[1] * 10 + rev[2]
        }
    )", "main");
    REQUIRE(result.as_int() == 321);  // [3, 2, 1]
}

TEST_CASE("VM: List.concat", "[vm][phase6][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let a = [1, 2]
            let b = [3, 4]
            let c = a.concat(b)
            return c.length()
        }
    )", "main");
    REQUIRE(result.as_int() == 4);
}

TEST_CASE("VM: List.concat - values preserved", "[vm][phase6][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let a = [10, 20]
            let b = [30, 40]
            let c = a.concat(b)
            return c[0] + c[1] + c[2] + c[3]
        }
    )", "main");
    REQUIRE(result.as_int() == 100);  // 10 + 20 + 30 + 40
}

TEST_CASE("VM: List method chaining", "[vm][phase6][list]") {
    auto result = execute_program(R"(
        function main() returns Int {
            let nums = [1, 2, 3]
            return nums.reverse().concat([0]).length()
        }
    )", "main");
    REQUIRE(result.as_int() == 4);  // [3, 2, 1, 0]
}

// ===== Numeric Methods Tests =====

TEST_CASE("VM: Int.to_string", "[vm][phase6][numeric]") {
    auto result = execute_program(R"(
        function main() returns String {
            return (42).to_string()
        }
    )", "main");
    REQUIRE(result.as_string() == "42");
}

TEST_CASE("VM: Int.abs - positive", "[vm][phase6][numeric]") {
    auto result = execute_program(R"(
        function main() returns Int {
            return (42).abs()
        }
    )", "main");
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Int.abs - negative", "[vm][phase6][numeric]") {
    auto result = execute_program(R"(
        function main() returns Int {
            return (0 - 42).abs()
        }
    )", "main");
    REQUIRE(result.as_int() == 42);
}

TEST_CASE("VM: Float.to_string", "[vm][phase6][numeric]") {
    auto result = execute_program(R"(
        function main() returns String {
            return (3.14).to_string()
        }
    )", "main");
    // Float formatting may vary, just check it's not empty
    REQUIRE(!result.as_string().empty());
    REQUIRE(result.as_string().find("3.14") != std::string::npos);
}

TEST_CASE("VM: Float.abs", "[vm][phase6][numeric]") {
    auto result = execute_program(R"(
        function main() returns Float {
            return (0.0 - 3.5).abs()
        }
    )", "main");
    REQUIRE(result.as_float() == 3.5);
}

TEST_CASE("VM: Float.floor", "[vm][phase6][numeric]") {
    SECTION("Positive") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (3.7).floor()
            }
        )", "main");
        REQUIRE(result.as_int() == 3);
    }

    SECTION("Negative") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (0.0 - 3.2).floor()
            }
        )", "main");
        REQUIRE(result.as_int() == -4);
    }
}

TEST_CASE("VM: Float.ceil", "[vm][phase6][numeric]") {
    SECTION("Positive") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (3.2).ceil()
            }
        )", "main");
        REQUIRE(result.as_int() == 4);
    }

    SECTION("Negative") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (0.0 - 3.7).ceil()
            }
        )", "main");
        REQUIRE(result.as_int() == -3);
    }
}

TEST_CASE("VM: Float.round", "[vm][phase6][numeric]") {
    SECTION("Round down") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (3.4).round()
            }
        )", "main");
        REQUIRE(result.as_int() == 3);
    }

    SECTION("Round up") {
        auto result = execute_program(R"(
            function main() returns Int {
                return (3.6).round()
            }
        )", "main");
        REQUIRE(result.as_int() == 4);
    }
}

// ===== Phase 7: File I/O Tests =====

#include <fstream>
#include <filesystem>

// Helper to create a temporary test file
auto create_temp_file(const std::string& content) -> std::string {
    std::string path = "/tmp/lucid_test_" + std::to_string(std::rand()) + ".txt";
    std::ofstream file(path);
    file << content;
    file.close();
    return path;
}

// Helper to read a file
auto read_test_file(const std::string& path) -> std::string {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper to delete test file
auto delete_test_file(const std::string& path) -> void {
    std::filesystem::remove(path);
}

TEST_CASE("VM: read_file - existing file", "[vm][phase7][fileio]") {
    std::string path = create_temp_file("Hello, File!");

    auto result = execute_program(fmt::format(R"(
        function main() returns String {{
            return read_file("{}")
        }}
    )", path), "main");

    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "Hello, File!");

    delete_test_file(path);
}

TEST_CASE("VM: read_file - non-existent file", "[vm][phase7][fileio]") {
    auto result = execute_program(R"(
        function main() returns String {
            return read_file("/nonexistent/file/path.txt")
        }
    )", "main");

    REQUIRE(result.is_string());
    REQUIRE(result.as_string() == "");  // Returns empty string on error
}

TEST_CASE("VM: write_file", "[vm][phase7][fileio]") {
    std::string path = "/tmp/lucid_write_test_" + std::to_string(std::rand()) + ".txt";

    auto result = execute_program(fmt::format(R"(
        function main() returns Bool {{
            return write_file("{}", "Written content")
        }}
    )", path), "main");

    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);

    // Verify content was written
    REQUIRE(read_test_file(path) == "Written content");

    delete_test_file(path);
}

TEST_CASE("VM: append_file", "[vm][phase7][fileio]") {
    std::string path = create_temp_file("Line 1\n");

    auto result = execute_program(fmt::format(R"(
        function main() returns Bool {{
            return append_file("{}", "Line 2\n")
        }}
    )", path), "main");

    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);

    // Verify content was appended
    REQUIRE(read_test_file(path) == "Line 1\nLine 2\n");

    delete_test_file(path);
}

TEST_CASE("VM: file_exists - existing file", "[vm][phase7][fileio]") {
    std::string path = create_temp_file("test");

    auto result = execute_program(fmt::format(R"(
        function main() returns Bool {{
            return file_exists("{}")
        }}
    )", path), "main");

    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);

    delete_test_file(path);
}

TEST_CASE("VM: file_exists - non-existent file", "[vm][phase7][fileio]") {
    auto result = execute_program(R"(
        function main() returns Bool {
            return file_exists("/nonexistent/file/path.txt")
        }
    )", "main");

    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == false);
}

TEST_CASE("VM: File I/O integration - read and write", "[vm][phase7][fileio]") {
    std::string src_path = create_temp_file("Source content");
    std::string dst_path = "/tmp/lucid_copy_test_" + std::to_string(std::rand()) + ".txt";

    auto result = execute_program(fmt::format(R"(
        function main() returns Bool {{
            let content = read_file("{}")
            return write_file("{}", content)
        }}
    )", src_path, dst_path), "main");

    REQUIRE(result.is_bool());
    REQUIRE(result.as_bool() == true);
    REQUIRE(read_test_file(dst_path) == "Source content");

    delete_test_file(src_path);
    delete_test_file(dst_path);
}
