#include <lucid/semantic/type_system.hpp>

// Support both system Catch2 and amalgamated version
#if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
#else
    #include <catch_amalgamated.hpp>
#endif

using namespace lucid::semantic;

// ===== Primitive Type Tests =====

TEST_CASE("Type System: Primitive types", "[type_system]") {
    auto int_type = PrimitiveType(PrimitiveKind::Int);
    auto float_type = PrimitiveType(PrimitiveKind::Float);
    auto string_type = PrimitiveType(PrimitiveKind::String);
    auto bool_type = PrimitiveType(PrimitiveKind::Bool);

    SECTION("to_string") {
        REQUIRE(int_type.to_string() == "Int");
        REQUIRE(float_type.to_string() == "Float");
        REQUIRE(string_type.to_string() == "String");
        REQUIRE(bool_type.to_string() == "Bool");
    }

    SECTION("equality") {
        auto another_int = PrimitiveType(PrimitiveKind::Int);
        REQUIRE(int_type.equals(another_int));
        REQUIRE(!int_type.equals(float_type));
        REQUIRE(!float_type.equals(string_type));
    }

    SECTION("clone") {
        auto cloned = int_type.clone();
        REQUIRE(cloned->to_string() == "Int");
        REQUIRE(int_type.equals(*cloned));
    }
}

// ===== List Type Tests =====

TEST_CASE("Type System: List types", "[type_system]") {
    auto int_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    auto list_of_int = ListType(std::move(int_type));

    SECTION("to_string") {
        REQUIRE(list_of_int.to_string() == "List[Int]");
    }

    SECTION("equality") {
        auto another_list_int = ListType(
            std::make_unique<PrimitiveType>(PrimitiveKind::Int)
        );
        REQUIRE(list_of_int.equals(another_list_int));

        auto list_of_float = ListType(
            std::make_unique<PrimitiveType>(PrimitiveKind::Float)
        );
        REQUIRE(!list_of_int.equals(list_of_float));
    }

    SECTION("nested lists") {
        auto list_of_list_int = ListType(
            std::make_unique<ListType>(
                std::make_unique<PrimitiveType>(PrimitiveKind::Int)
            )
        );
        REQUIRE(list_of_list_int.to_string() == "List[List[Int]]");
    }
}

// ===== Tuple Type Tests =====

TEST_CASE("Type System: Tuple types", "[type_system]") {
    std::vector<std::unique_ptr<SemanticType>> elems;
    elems.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
    elems.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::String));

    auto tuple_type = TupleType(std::move(elems));

    SECTION("to_string") {
        REQUIRE(tuple_type.to_string() == "(Int, String)");
    }

    SECTION("empty tuple") {
        auto empty = TupleType(std::vector<std::unique_ptr<SemanticType>>{});
        REQUIRE(empty.to_string() == "()");
    }

    SECTION("equality") {
        std::vector<std::unique_ptr<SemanticType>> elems2;
        elems2.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
        elems2.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::String));
        auto another_tuple = TupleType(std::move(elems2));

        REQUIRE(tuple_type.equals(another_tuple));

        // Different element types
        std::vector<std::unique_ptr<SemanticType>> elems3;
        elems3.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
        elems3.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
        auto different_tuple = TupleType(std::move(elems3));

        REQUIRE(!tuple_type.equals(different_tuple));
    }
}

// ===== Function Type Tests =====

TEST_CASE("Type System: Function types", "[type_system]") {
    std::vector<std::unique_ptr<SemanticType>> params;
    params.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
    params.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));

    auto return_type = std::make_unique<PrimitiveType>(PrimitiveKind::Int);

    auto func_type = FunctionType(std::move(params), std::move(return_type));

    SECTION("to_string") {
        REQUIRE(func_type.to_string() == "(Int, Int) -> Int");
    }

    SECTION("no parameters") {
        auto no_param_func = FunctionType(
            std::vector<std::unique_ptr<SemanticType>>{},
            std::make_unique<PrimitiveType>(PrimitiveKind::Bool)
        );
        REQUIRE(no_param_func.to_string() == "() -> Bool");
    }

    SECTION("equality") {
        std::vector<std::unique_ptr<SemanticType>> params2;
        params2.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));
        params2.push_back(std::make_unique<PrimitiveType>(PrimitiveKind::Int));

        auto func_type2 = FunctionType(
            std::move(params2),
            std::make_unique<PrimitiveType>(PrimitiveKind::Int)
        );

        REQUIRE(func_type.equals(func_type2));
    }
}

// ===== Type Variable Tests =====

TEST_CASE("Type System: Type variables", "[type_system]") {
    auto type_var = TypeVariable("'a");

    SECTION("to_string") {
        REQUIRE(type_var.to_string() == "'a");
    }

    SECTION("equality") {
        auto another_a = TypeVariable("'a");
        auto type_b = TypeVariable("'b");

        REQUIRE(type_var.equals(another_a));
        REQUIRE(!type_var.equals(type_b));
    }
}

// ===== Unknown Type Tests =====

TEST_CASE("Type System: Unknown type", "[type_system]") {
    auto unknown1 = UnknownType();
    auto unknown2 = UnknownType();

    SECTION("to_string") {
        REQUIRE(unknown1.to_string() == "?");
    }

    SECTION("never equals") {
        REQUIRE(!unknown1.equals(unknown2));
        auto int_type = PrimitiveType(PrimitiveKind::Int);
        REQUIRE(!unknown1.equals(int_type));
    }
}

// ===== Type Operations Tests =====

TEST_CASE("Type System: Type compatibility", "[type_system]") {
    auto int1 = PrimitiveType(PrimitiveKind::Int);
    auto int2 = PrimitiveType(PrimitiveKind::Int);
    auto float_type = PrimitiveType(PrimitiveKind::Float);

    SECTION("compatible same types") {
        REQUIRE(types_compatible(int1, int2));
    }

    SECTION("incompatible different types") {
        REQUIRE(!types_compatible(int1, float_type));
    }
}

TEST_CASE("Type System: Type unification", "[type_system]") {
    auto int_type = PrimitiveType(PrimitiveKind::Int);
    auto float_type = PrimitiveType(PrimitiveKind::Float);

    SECTION("unify same types") {
        auto result = unify_types(int_type, int_type);
        REQUIRE(result.has_value());
        REQUIRE((*result)->to_string() == "Int");
    }

    SECTION("unify different types fails") {
        auto result = unify_types(int_type, float_type);
        REQUIRE(!result.has_value());
    }

    SECTION("unify with type variable") {
        auto type_var = TypeVariable("'a");
        auto result = unify_types(type_var, int_type);
        REQUIRE(result.has_value());
        REQUIRE((*result)->to_string() == "Int");

        auto result2 = unify_types(int_type, type_var);
        REQUIRE(result2.has_value());
        REQUIRE((*result2)->to_string() == "Int");
    }

    SECTION("unify with unknown") {
        auto unknown = UnknownType();
        auto result = unify_types(unknown, int_type);
        REQUIRE(result.has_value());
        REQUIRE((*result)->to_string() == "?");
    }
}

// ===== Type Environment Tests =====

TEST_CASE("Type System: Type environment", "[type_system]") {
    TypeEnvironment env;

    SECTION("get builtin types") {
        auto int_type = env.get_builtin("Int");
        REQUIRE(int_type.has_value());
        REQUIRE((*int_type)->to_string() == "Int");

        auto float_type = env.get_builtin("Float");
        REQUIRE(float_type.has_value());
        REQUIRE((*float_type)->to_string() == "Float");

        auto string_type = env.get_builtin("String");
        REQUIRE(string_type.has_value());
        REQUIRE((*string_type)->to_string() == "String");

        auto bool_type = env.get_builtin("Bool");
        REQUIRE(bool_type.has_value());
        REQUIRE((*bool_type)->to_string() == "Bool");
    }

    SECTION("non-builtin returns nullopt") {
        auto custom = env.get_builtin("MyType");
        REQUIRE(!custom.has_value());
    }

    SECTION("is_builtin check") {
        REQUIRE(env.is_builtin("Int"));
        REQUIRE(env.is_builtin("Float"));
        REQUIRE(env.is_builtin("String"));
        REQUIRE(env.is_builtin("Bool"));
        REQUIRE(!env.is_builtin("MyType"));
    }
}
