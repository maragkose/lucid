#include <lucid/semantic/symbol_table.hpp>

// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include <catch_amalgamated.hpp>
#endif

using namespace lucid::semantic;
using namespace lucid;

// Helper to create a source location
auto make_location() -> SourceLocation {
    return SourceLocation("test.lucid", 1, 1, 0, 0);
}

// ===== Symbol Tests =====

TEST_CASE("Symbol Table: Symbol creation", "[symbol_table]") {
    auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    auto symbol = Symbol("x", SymbolKind::Variable, std::move(int_type),
                        make_location(), false);

    SECTION("symbol properties") {
        REQUIRE(symbol.name == "x");
        REQUIRE(symbol.kind == SymbolKind::Variable);
        REQUIRE(symbol.type->to_string() == "Int");
        REQUIRE(symbol.is_mutable == false);
    }
}

// ===== Scope Tests =====

TEST_CASE("Symbol Table: Scope operations", "[symbol_table]") {
    Scope scope(Scope::ScopeKind::Global);

    SECTION("declare and lookup") {
        auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        bool success = scope.declare("x", SymbolKind::Variable,
                                    std::move(int_type), make_location());
        REQUIRE(success);

        auto* symbol = scope.lookup_local("x");
        REQUIRE(symbol != nullptr);
        REQUIRE(symbol->name == "x");
        REQUIRE(symbol->type->to_string() == "Int");
    }

    SECTION("redeclaration fails") {
        auto int_type1 = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        auto int_type2 = std::make_unique<PrimitiveType>(PrimitiveKind::Int);

        bool first = scope.declare("x", SymbolKind::Variable,
                                  std::move(int_type1), make_location());
        REQUIRE(first);

        bool second = scope.declare("x", SymbolKind::Variable,
                                   std::move(int_type2), make_location());
        REQUIRE(!second);  // Should fail
    }

    SECTION("exists_local") {
        auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        scope.declare("x", SymbolKind::Variable, std::move(int_type),
                     make_location());

        REQUIRE(scope.exists_local("x"));
        REQUIRE(!scope.exists_local("y"));
    }

    SECTION("lookup non-existent returns nullptr") {
        auto* symbol = scope.lookup_local("nonexistent");
        REQUIRE(symbol == nullptr);
    }
}

// ===== SymbolTable Tests =====

TEST_CASE("Symbol Table: Global scope", "[symbol_table]") {
    SymbolTable table;

    SECTION("starts with global scope") {
        REQUIRE(table.scope_depth() == 0);
        REQUIRE(table.current_scope()->kind == Scope::ScopeKind::Global);
    }

    SECTION("declare in global scope") {
        auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        bool success = table.declare("x", SymbolKind::Variable,
                                    std::move(int_type), make_location());
        REQUIRE(success);

        auto* symbol = table.lookup("x");
        REQUIRE(symbol != nullptr);
        REQUIRE(symbol->name == "x");
    }

    SECTION("redeclaration in global scope fails") {
        auto int_type1 = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        auto int_type2 = std::make_unique<PrimitiveType>(PrimitiveKind::Int);

        bool first = table.declare("x", SymbolKind::Variable,
                                  std::move(int_type1), make_location());
        REQUIRE(first);

        bool second = table.declare("x", SymbolKind::Variable,
                                   std::move(int_type2), make_location());
        REQUIRE(!second);
    }
}

TEST_CASE("Symbol Table: Nested scopes", "[symbol_table]") {
    SymbolTable table;

    SECTION("enter and exit scope") {
        REQUIRE(table.scope_depth() == 0);

        table.enter_scope(Scope::ScopeKind::Function);
        REQUIRE(table.scope_depth() == 1);
        REQUIRE(table.current_scope()->kind == Scope::ScopeKind::Function);

        table.enter_scope(Scope::ScopeKind::Block);
        REQUIRE(table.scope_depth() == 2);
        REQUIRE(table.current_scope()->kind == Scope::ScopeKind::Block);

        table.exit_scope();
        REQUIRE(table.scope_depth() == 1);

        table.exit_scope();
        REQUIRE(table.scope_depth() == 0);
    }

    SECTION("lookup in nested scopes") {
        // Declare in global scope
        auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        table.declare("x", SymbolKind::Variable, std::move(int_type),
                     make_location());

        // Enter function scope
        table.enter_scope(Scope::ScopeKind::Function);

        // Can still lookup global variable
        auto* symbol = table.lookup("x");
        REQUIRE(symbol != nullptr);
        REQUIRE(symbol->name == "x");

        // Declare local variable
        auto float_type = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
        table.declare("y", SymbolKind::Variable, std::move(float_type),
                     make_location());

        // Can lookup local variable
        auto* local = table.lookup("y");
        REQUIRE(local != nullptr);
        REQUIRE(local->type->to_string() == "Float");

        // Exit scope
        table.exit_scope();

        // Local variable no longer accessible
        auto* gone = table.lookup("y");
        REQUIRE(gone == nullptr);

        // Global variable still accessible
        auto* global = table.lookup("x");
        REQUIRE(global != nullptr);
    }
}

TEST_CASE("Symbol Table: Shadowing", "[symbol_table]") {
    SymbolTable table;

    // Declare x in global scope as Int
    auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    table.declare("x", SymbolKind::Variable, std::move(int_type),
                 make_location());

    auto* global_x = table.lookup("x");
    REQUIRE(global_x->type->to_string() == "Int");

    // Enter function scope
    table.enter_scope(Scope::ScopeKind::Function);

    // Shadow x with Float type
    auto float_type = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
    bool success = table.declare("x", SymbolKind::Variable,
                                std::move(float_type), make_location());
    REQUIRE(success);  // Should succeed (shadowing is allowed)

    // Lookup finds the shadowing variable
    auto* local_x = table.lookup("x");
    REQUIRE(local_x->type->to_string() == "Float");

    // Exit scope
    table.exit_scope();

    // Lookup finds the original global variable again
    auto* original_x = table.lookup("x");
    REQUIRE(original_x->type->to_string() == "Int");
}

TEST_CASE("Symbol Table: exists and exists_in_current_scope", "[symbol_table]") {
    SymbolTable table;

    auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    table.declare("x", SymbolKind::Variable, std::move(int_type),
                 make_location());

    SECTION("exists in global scope") {
        REQUIRE(table.exists("x"));
        REQUIRE(table.exists_in_current_scope("x"));
        REQUIRE(!table.exists("y"));
    }

    SECTION("exists through parent scope") {
        table.enter_scope(Scope::ScopeKind::Function);

        // x exists through parent scope
        REQUIRE(table.exists("x"));
        // but not in current scope
        REQUIRE(!table.exists_in_current_scope("x"));

        // Declare y in function scope
        auto float_type = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
        table.declare("y", SymbolKind::Variable, std::move(float_type),
                     make_location());

        // y exists in current scope
        REQUIRE(table.exists("y"));
        REQUIRE(table.exists_in_current_scope("y"));

        table.exit_scope();

        // Back in global scope, y no longer exists
        REQUIRE(!table.exists("y"));
    }
}

TEST_CASE("Symbol Table: Function symbols", "[symbol_table]") {
    SymbolTable table;

    SECTION("make_function_symbol helper") {
        std::vector<std::unique_ptr<SemanticType>> params;
        params.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
        params.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));

        auto return_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);

        auto func_symbol = make_function_symbol(
            "add",
            std::move(params),
            std::move(return_type),
            make_location()
        );

        REQUIRE(func_symbol->name == "add");
        REQUIRE(func_symbol->kind == SymbolKind::Function);
        REQUIRE(func_symbol->type->to_string() == "(Int, Int) -> Int");
        REQUIRE(!func_symbol->is_mutable);
    }

    SECTION("declare function in symbol table") {
        std::vector<std::unique_ptr<SemanticType>> params;
        params.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));

        auto func_type = std::make_unique<FunctionType>(
            std::move(params),
            std::make_unique<PrimitiveType>(PrimitiveKind::Bool)
        );

        bool success = table.declare("is_positive", SymbolKind::Function,
                                    std::move(func_type), make_location());
        REQUIRE(success);

        auto* func = table.lookup("is_positive");
        REQUIRE(func != nullptr);
        REQUIRE(func->kind == SymbolKind::Function);
        REQUIRE(func->type->to_string() == "(Int) -> Bool");
    }
}

TEST_CASE("Symbol Table: Multiple scope kinds", "[symbol_table]") {
    SymbolTable table;

    // Global scope
    auto global_var = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    table.declare("global_x", SymbolKind::Variable, std::move(global_var),
                 make_location());

    // Function scope
    table.enter_scope(Scope::ScopeKind::Function);
    auto func_var = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
    table.declare("func_y", SymbolKind::Variable, std::move(func_var),
                 make_location());

    // Block scope inside function
    table.enter_scope(Scope::ScopeKind::Block);
    auto block_var = std::make_unique<PrimitiveType>(PrimitiveKind::String);
    table.declare("block_z", SymbolKind::Variable, std::move(block_var),
                 make_location());

    // Lambda scope inside block
    table.enter_scope(Scope::ScopeKind::Lambda);
    auto lambda_var = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
    table.declare("lambda_w", SymbolKind::Variable, std::move(lambda_var),
                 make_location());

    SECTION("all variables accessible from deepest scope") {
        REQUIRE(table.scope_depth() == 3);
        REQUIRE(table.exists("global_x"));
        REQUIRE(table.exists("func_y"));
        REQUIRE(table.exists("block_z"));
        REQUIRE(table.exists("lambda_w"));
    }

    // Exit lambda scope
    table.exit_scope();
    REQUIRE(table.scope_depth() == 2);
    REQUIRE(!table.exists("lambda_w"));

    // Exit block scope
    table.exit_scope();
    REQUIRE(table.scope_depth() == 1);
    REQUIRE(!table.exists("block_z"));

    // Exit function scope
    table.exit_scope();
    REQUIRE(table.scope_depth() == 0);
    REQUIRE(!table.exists("func_y"));
    REQUIRE(table.exists("global_x"));
}
