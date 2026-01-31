#include <lucid/semantic/type_checker.hpp>
#include <fmt/format.h>

namespace lucid {
namespace semantic {

// ===== TypeChecker Construction =====

TypeChecker::TypeChecker()
    : current_function_return_type_(nullptr) {}

// ===== Main Entry Points =====

auto TypeChecker::check_program(ast::Program& program) -> TypeCheckResult {
    // First pass: collect all function signatures
    for (auto& func : program.functions) {
        // Convert parameter types
        std::vector<std::unique_ptr<SemanticType>> param_types;
        for (auto& param : func->parameters) {
            param_types.push_back(ast_type_to_semantic(*param->type));
        }

        // Convert return type
        auto return_type = ast_type_to_semantic(*func->return_type);

        // Create function symbol
        auto func_type = std::make_unique<FunctionType>(
            std::move(param_types),
            return_type->clone()
        );

        // Declare function in global scope
        bool success = symbol_table_.declare(
            func->name,
            SymbolKind::Function,
            std::move(func_type),
            func->location
        );

        if (!success) {
            error(func->location,
                  fmt::format("Function '{}' is already declared", func->name));
        }
    }

    // Second pass: type check function bodies
    for (auto& func : program.functions) {
        check_function(*func);
    }

    return std::move(result_);
}

auto TypeChecker::check_function(ast::FunctionDef& func) -> void {
    // Enter function scope
    symbol_table_.enter_scope(Scope::ScopeKind::Function);

    // Set current function return type
    auto return_type = ast_type_to_semantic(*func.return_type);
    current_function_return_type_ = return_type.get();

    // Declare parameters in function scope
    for (auto& param : func.parameters) {
        auto param_type = ast_type_to_semantic(*param->type);
        bool success = symbol_table_.declare(
            param->name,
            SymbolKind::Parameter,
            std::move(param_type),
            param->location
        );

        if (!success) {
            error(param->location,
                  fmt::format("Parameter '{}' is already declared", param->name));
        }
    }

    // Type check function body
    // Note: We don't check the body type against the return type here, because
    // functions use explicit return statements. The return type is validated
    // in visit_return() for each return statement.
    check_expression(*func.body);

    // Exit function scope
    symbol_table_.exit_scope();
    current_function_return_type_ = nullptr;
}

auto TypeChecker::check_expression(ast::Expr& expr) -> std::unique_ptr<SemanticType> {
    // Visit the expression (sets current_type_)
    expr.accept(*this);

    // Return the type (or Unknown if there was an error)
    if (current_type_) {
        return std::move(current_type_);
    }
    return std::make_unique<UnknownType>();
}

auto TypeChecker::check_statement(ast::Stmt& stmt) -> void {
    stmt.accept(*this);
}

// ===== Expression Visitors =====

auto TypeChecker::visit_int_literal(ast::IntLiteralExpr* /* expr */) -> void {
    current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
}

auto TypeChecker::visit_float_literal(ast::FloatLiteralExpr* /* expr */) -> void {
    current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
}

auto TypeChecker::visit_string_literal(ast::StringLiteralExpr* /* expr */) -> void {
    current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
}

auto TypeChecker::visit_bool_literal(ast::BoolLiteralExpr* /* expr */) -> void {
    current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
}

auto TypeChecker::visit_identifier(ast::IdentifierExpr* expr) -> void {
    // Lookup identifier in symbol table
    auto* symbol = symbol_table_.lookup(expr->name);

    if (!symbol) {
        error(expr->location, fmt::format("Undefined variable '{}'", expr->name));
        current_type_ = std::make_unique<UnknownType>();
        return;
    }

    // Return a clone of the symbol's type
    current_type_ = symbol->type->clone();
}

auto TypeChecker::visit_tuple(ast::TupleExpr* expr) -> void {
    std::vector<std::unique_ptr<SemanticType>> elem_types;

    for (auto& elem : expr->elements) {
        auto elem_type = check_expression(*elem);
        elem_types.push_back(std::move(elem_type));
    }

    current_type_ = std::make_unique<TupleType>(std::move(elem_types));
}

auto TypeChecker::visit_list(ast::ListExpr* expr) -> void {
    if (expr->elements.empty()) {
        // Empty list - infer as List[Unknown]
        current_type_ = std::make_unique<ListType>(
            std::make_unique<UnknownType>()
        );
        return;
    }

    // Check first element to get element type
    auto first_type = check_expression(*expr->elements[0]);

    // Check remaining elements have same type
    for (size_t i = 1; i < expr->elements.size(); ++i) {
        auto elem_type = check_expression(*expr->elements[i]);
        if (!elem_type->equals(*first_type)) {
            type_mismatch_error(expr->elements[i]->location, *first_type, *elem_type);
        }
    }

    current_type_ = std::make_unique<ListType>(std::move(first_type));
}

auto TypeChecker::visit_binary(ast::BinaryExpr* expr) -> void {
    using Op = ast::BinaryOp;

    switch (expr->op) {
        // Arithmetic operators
        case Op::Add:
        case Op::Sub:
        case Op::Mul:
        case Op::Div:
        case Op::Mod:
        case Op::Pow:
            current_type_ = check_binary_arithmetic(expr);
            break;

        // Comparison operators
        case Op::Eq:
        case Op::Ne:
        case Op::Lt:
        case Op::Gt:
        case Op::Le:
        case Op::Ge:
            current_type_ = check_binary_comparison(expr);
            break;

        // Logical operators
        case Op::And:
        case Op::Or:
            current_type_ = check_binary_logical(expr);
            break;
    }
}

auto TypeChecker::visit_unary(ast::UnaryExpr* expr) -> void {
    using Op = ast::UnaryOp;

    switch (expr->op) {
        case Op::Pos:
        case Op::Neg:
            current_type_ = check_unary_arithmetic(expr);
            break;

        case Op::Not:
            current_type_ = check_unary_logical(expr);
            break;
    }
}

auto TypeChecker::visit_call(ast::CallExpr* expr) -> void {
    // For now, we only support calling identifiers (function names)
    // In the future, we could support calling lambdas or function expressions
    if (expr->callee->kind != ast::ExprKind::Identifier) {
        error(expr->location, "Only function names can be called for now");
        current_type_ = std::make_unique<UnknownType>();
        return;
    }

    auto* identifier = static_cast<ast::IdentifierExpr*>(expr->callee.get());
    std::string func_name = identifier->name;

    // Check for built-in functions first
    if (func_name == "print" || func_name == "println") {
        // print(value) -> Unit, println(value) -> Unit
        // Accept any type as argument
        if (expr->arguments.size() != 1) {
            error(expr->location,
                  fmt::format("Function '{}' expects 1 argument, got {}",
                             func_name, expr->arguments.size()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }
        // Type check the argument (any type is valid)
        check_expression(*expr->arguments[0]);
        // Return Int as placeholder for Unit (print returns nothing meaningful)
        current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        return;
    }

    if (func_name == "to_string") {
        // to_string(value) -> String
        // Accept any type as argument
        if (expr->arguments.size() != 1) {
            error(expr->location,
                  fmt::format("Function 'to_string' expects 1 argument, got {}",
                             expr->arguments.size()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }
        // Type check the argument (any type is valid)
        check_expression(*expr->arguments[0]);
        // Return String
        current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
        return;
    }

    if (func_name == "read_file") {
        // read_file(path: String) -> String
        if (expr->arguments.size() != 1) {
            error(expr->location,
                  fmt::format("Function 'read_file' expects 1 argument, got {}",
                             expr->arguments.size()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }
        auto arg_type = check_expression(*expr->arguments[0]);
        if (arg_type->kind != TypeKind::Primitive ||
            static_cast<PrimitiveType*>(arg_type.get())->primitive_kind != PrimitiveKind::String) {
            type_mismatch_error(expr->arguments[0]->location,
                              PrimitiveType(PrimitiveKind::String), *arg_type);
        }
        current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
        return;
    }

    if (func_name == "write_file" || func_name == "append_file") {
        // write_file(path: String, content: String) -> Bool
        // append_file(path: String, content: String) -> Bool
        if (expr->arguments.size() != 2) {
            error(expr->location,
                  fmt::format("Function '{}' expects 2 arguments, got {}",
                             func_name, expr->arguments.size()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }
        // Check both arguments are strings
        for (size_t i = 0; i < 2; ++i) {
            auto arg_type = check_expression(*expr->arguments[i]);
            if (arg_type->kind != TypeKind::Primitive ||
                static_cast<PrimitiveType*>(arg_type.get())->primitive_kind != PrimitiveKind::String) {
                type_mismatch_error(expr->arguments[i]->location,
                                  PrimitiveType(PrimitiveKind::String), *arg_type);
            }
        }
        current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
        return;
    }

    if (func_name == "file_exists") {
        // file_exists(path: String) -> Bool
        if (expr->arguments.size() != 1) {
            error(expr->location,
                  fmt::format("Function 'file_exists' expects 1 argument, got {}",
                             expr->arguments.size()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }
        auto arg_type = check_expression(*expr->arguments[0]);
        if (arg_type->kind != TypeKind::Primitive ||
            static_cast<PrimitiveType*>(arg_type.get())->primitive_kind != PrimitiveKind::String) {
            type_mismatch_error(expr->arguments[0]->location,
                              PrimitiveType(PrimitiveKind::String), *arg_type);
        }
        current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
        return;
    }

    // Lookup function in symbol table
    auto* func_symbol = symbol_table_.lookup(func_name);

    if (!func_symbol) {
        error(expr->location, fmt::format("Undefined function '{}'", func_name));
        current_type_ = std::make_unique<UnknownType>();
        return;
    }

    // Function symbol must have function type
    if (func_symbol->type->kind != TypeKind::Function) {
        error(expr->location, fmt::format("'{}' is not a function", func_name));
        current_type_ = std::make_unique<UnknownType>();
        return;
    }

    auto* func_type = static_cast<FunctionType*>(func_symbol->type.get());

    // Check argument count
    if (expr->arguments.size() != func_type->param_types.size()) {
        error(expr->location,
              fmt::format("Function '{}' expects {} arguments, got {}",
                         func_name,
                         func_type->param_types.size(),
                         expr->arguments.size()));
        current_type_ = std::make_unique<UnknownType>();
        return;
    }

    // Type check each argument
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        auto arg_type = check_expression(*expr->arguments[i]);
        auto& param_type = func_type->param_types[i];

        if (!arg_type->equals(*param_type)) {
            type_mismatch_error(expr->arguments[i]->location, *param_type, *arg_type);
        }
    }

    // Return type is function's return type
    current_type_ = func_type->return_type->clone();
}

auto TypeChecker::visit_method_call(ast::MethodCallExpr* expr) -> void {
    // Type check the object
    auto object_type = check_expression(*expr->object);

    // Check methods based on object type
    if (object_type->kind == TypeKind::List) {
        auto* list_type = static_cast<ListType*>(object_type.get());

        if (expr->method_name == "append") {
            // append(element) -> List[T] (immutable, returns new list)
            if (expr->arguments.size() != 1) {
                error(expr->location,
                      fmt::format("Method 'append' expects 1 argument, got {}",
                                 expr->arguments.size()));
                current_type_ = std::make_unique<UnknownType>();
                return;
            }

            auto arg_type = check_expression(*expr->arguments[0]);
            if (!arg_type->equals(*list_type->element_type)) {
                type_mismatch_error(expr->arguments[0]->location,
                                   *list_type->element_type, *arg_type);
            }

            // Returns a new list with the same element type
            current_type_ = std::make_unique<ListType>(list_type->element_type->clone());
        } else if (expr->method_name == "length") {
            // length() -> Int
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'length' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        } else if (expr->method_name == "head") {
            // head() -> T (first element)
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'head' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = list_type->element_type->clone();
        } else if (expr->method_name == "tail") {
            // tail() -> List[T] (all elements except first)
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'tail' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = std::make_unique<ListType>(list_type->element_type->clone());
        } else if (expr->method_name == "is_empty") {
            // is_empty() -> Bool
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'is_empty' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
        } else if (expr->method_name == "reverse") {
            // reverse() -> List[T]
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'reverse' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = std::make_unique<ListType>(list_type->element_type->clone());
        } else if (expr->method_name == "concat") {
            // concat(other: List[T]) -> List[T]
            if (expr->arguments.size() != 1) {
                error(expr->location,
                      fmt::format("Method 'concat' expects 1 argument, got {}",
                                 expr->arguments.size()));
                current_type_ = std::make_unique<UnknownType>();
            } else {
                auto arg_type = check_expression(*expr->arguments[0]);
                if (arg_type->kind != TypeKind::List) {
                    error(expr->arguments[0]->location,
                          fmt::format("Method 'concat' expects List argument, got {}",
                                     arg_type->to_string()));
                } else {
                    auto* arg_list_type = static_cast<ListType*>(arg_type.get());
                    if (!arg_list_type->element_type->equals(*list_type->element_type)) {
                        type_mismatch_error(expr->arguments[0]->location,
                                          *list_type, *arg_type);
                    }
                }
                current_type_ = std::make_unique<ListType>(list_type->element_type->clone());
            }
        } else {
            error(expr->location,
                  fmt::format("List type has no method '{}'", expr->method_name));
            current_type_ = std::make_unique<UnknownType>();
        }
    } else if (object_type->kind == TypeKind::Primitive) {
        auto* prim_type = static_cast<PrimitiveType*>(object_type.get());

        if (prim_type->primitive_kind == PrimitiveKind::String) {
            if (expr->method_name == "length") {
                // length() -> Int
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'length' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
            } else if (expr->method_name == "is_empty") {
                // is_empty() -> Bool
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'is_empty' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
            } else if (expr->method_name == "contains" ||
                       expr->method_name == "starts_with" ||
                       expr->method_name == "ends_with") {
                // contains/starts_with/ends_with(substring: String) -> Bool
                if (expr->arguments.size() != 1) {
                    error(expr->location,
                          fmt::format("Method '{}' expects 1 argument, got {}",
                                     expr->method_name, expr->arguments.size()));
                } else {
                    auto arg_type = check_expression(*expr->arguments[0]);
                    if (arg_type->kind != TypeKind::Primitive ||
                        static_cast<PrimitiveType*>(arg_type.get())->primitive_kind != PrimitiveKind::String) {
                        type_mismatch_error(expr->arguments[0]->location,
                                          PrimitiveType(PrimitiveKind::String), *arg_type);
                    }
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
            } else if (expr->method_name == "to_upper" ||
                       expr->method_name == "to_lower" ||
                       expr->method_name == "trim") {
                // to_upper/to_lower/trim() -> String
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method '{}' expects 0 arguments, got {}",
                                     expr->method_name, expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
            } else {
                error(expr->location,
                      fmt::format("String type has no method '{}'", expr->method_name));
                current_type_ = std::make_unique<UnknownType>();
            }
        } else if (prim_type->primitive_kind == PrimitiveKind::Int) {
            if (expr->method_name == "to_string") {
                // to_string() -> String
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'to_string' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
            } else if (expr->method_name == "abs") {
                // abs() -> Int
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'abs' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
            } else {
                error(expr->location,
                      fmt::format("Int type has no method '{}'", expr->method_name));
                current_type_ = std::make_unique<UnknownType>();
            }
        } else if (prim_type->primitive_kind == PrimitiveKind::Float) {
            if (expr->method_name == "to_string") {
                // to_string() -> String
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'to_string' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::String);
            } else if (expr->method_name == "abs") {
                // abs() -> Float
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method 'abs' expects 0 arguments, got {}",
                                     expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Float);
            } else if (expr->method_name == "floor" ||
                       expr->method_name == "ceil" ||
                       expr->method_name == "round") {
                // floor/ceil/round() -> Int
                if (!expr->arguments.empty()) {
                    error(expr->location,
                          fmt::format("Method '{}' expects 0 arguments, got {}",
                                     expr->method_name, expr->arguments.size()));
                }
                current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
            } else {
                error(expr->location,
                      fmt::format("Float type has no method '{}'", expr->method_name));
                current_type_ = std::make_unique<UnknownType>();
            }
        } else {
            error(expr->location,
                  fmt::format("Type '{}' has no methods", object_type->to_string()));
            current_type_ = std::make_unique<UnknownType>();
        }
    } else if (object_type->kind == TypeKind::Tuple) {
        if (expr->method_name == "length") {
            // length() -> Int
            if (!expr->arguments.empty()) {
                error(expr->location,
                      fmt::format("Method 'length' expects 0 arguments, got {}",
                                 expr->arguments.size()));
            }
            current_type_ = std::make_unique<PrimitiveType>(PrimitiveKind::Int);
        } else {
            error(expr->location,
                  fmt::format("Tuple type has no method '{}'", expr->method_name));
            current_type_ = std::make_unique<UnknownType>();
        }
    } else {
        error(expr->location,
              fmt::format("Type '{}' has no methods", object_type->to_string()));
        current_type_ = std::make_unique<UnknownType>();
    }
}

auto TypeChecker::visit_index(ast::IndexExpr* expr) -> void {
    // Type check the object being indexed
    auto object_type = check_expression(*expr->object);

    // Type check the index expression
    auto index_type = check_expression(*expr->index);

    // Index must be Int
    auto int_type = PrimitiveType(PrimitiveKind::Int);
    if (!index_type->equals(int_type)) {
        type_mismatch_error(expr->index->location, int_type, *index_type);
    }

    // Check what we're indexing
    if (object_type->kind == TypeKind::List) {
        // List[T][Int] -> T
        auto* list_type = static_cast<ListType*>(object_type.get());
        current_type_ = list_type->element_type->clone();
    } else if (object_type->kind == TypeKind::Tuple) {
        auto* tuple_type = static_cast<TupleType*>(object_type.get());

        // Check if index is a constant integer literal
        if (expr->index->kind == ast::ExprKind::IntLiteral) {
            auto* int_literal = static_cast<ast::IntLiteralExpr*>(expr->index.get());
            int64_t index = int_literal->value;

            // Check bounds
            if (index < 0 || static_cast<size_t>(index) >= tuple_type->element_types.size()) {
                error(expr->index->location,
                      fmt::format("Tuple index {} out of bounds (tuple has {} elements)",
                                 index, tuple_type->element_types.size()));
                current_type_ = std::make_unique<UnknownType>();
            } else {
                // Return the type of the indexed element
                current_type_ = tuple_type->element_types[static_cast<size_t>(index)]->clone();
            }
        } else {
            error(expr->location,
                  "Tuple indexing requires a constant integer literal index");
            current_type_ = std::make_unique<UnknownType>();
        }
    } else {
        error(expr->object->location,
              fmt::format("Cannot index into type '{}'", object_type->to_string()));
        current_type_ = std::make_unique<UnknownType>();
    }
}

auto TypeChecker::visit_lambda(ast::LambdaExpr* expr) -> void {
    // Enter lambda scope
    symbol_table_.enter_scope(Scope::ScopeKind::Lambda);

    // For now, we'll use Unknown for parameter types since we don't have
    // type annotations on lambda parameters and full type inference is complex
    std::vector<std::unique_ptr<SemanticType>> param_types;

    // Declare lambda parameters with Unknown type
    for (auto& param : expr->parameters) {
        auto param_type = std::make_unique<UnknownType>();
        param_types.push_back(param_type->clone());

        symbol_table_.declare(
            param,
            SymbolKind::Parameter,
            std::move(param_type),
            expr->location
        );
    }

    // Type check lambda body
    auto body_type = check_expression(*expr->body);

    // Exit lambda scope
    symbol_table_.exit_scope();

    // Create function type for the lambda
    current_type_ = std::make_unique<FunctionType>(
        std::move(param_types),
        std::move(body_type)
    );
}

auto TypeChecker::visit_if(ast::IfExpr* expr) -> void {
    // Type check condition - must be Bool
    auto cond_type = check_expression(*expr->condition);
    auto bool_type = PrimitiveType(PrimitiveKind::Bool);

    if (!cond_type->equals(bool_type)) {
        type_mismatch_error(expr->condition->location, bool_type, *cond_type);
    }

    // Type check then branch
    auto then_type = check_expression(*expr->then_branch);

    // Type check else branch if present
    if (expr->else_branch.has_value()) {
        auto else_type = check_expression(**expr->else_branch);

        // Both branches must have compatible types
        if (!then_type->equals(*else_type)) {
            error((*expr->else_branch)->location,
                  fmt::format("If expression branches have incompatible types: '{}' and '{}'",
                             then_type->to_string(), else_type->to_string()));
            current_type_ = std::make_unique<UnknownType>();
            return;
        }

        current_type_ = std::move(then_type);
    } else {
        // If without else - result type is then_type but could be Unit
        // For now, we'll use the then type
        current_type_ = std::move(then_type);
    }
}

auto TypeChecker::visit_block(ast::BlockExpr* expr) -> void {
    // Enter a new scope for the block
    symbol_table_.enter_scope(Scope::ScopeKind::Block);

    // Type check all statements in the block
    for (auto& stmt : expr->statements) {
        check_statement(*stmt);
    }

    // If the last statement is an ExprStmt, the block has that expression's type
    // Otherwise, the block type is Unit (represented as Unknown for now)
    if (!expr->statements.empty()) {
        auto& last_stmt = expr->statements.back();
        if (last_stmt->kind == ast::StmtKind::ExprStmt) {
            auto* expr_stmt = static_cast<ast::ExprStmt*>(last_stmt.get());
            current_type_ = check_expression(*expr_stmt->expression);
        } else {
            // Block ends with non-expression statement, has Unit type
            current_type_ = std::make_unique<UnknownType>();
        }
    } else {
        // Empty block has Unit type
        current_type_ = std::make_unique<UnknownType>();
    }

    // Exit block scope
    symbol_table_.exit_scope();
}

// ===== Statement Visitors =====

auto TypeChecker::visit_let(ast::LetStmt* stmt) -> void {
    // Type check the initializer expression
    auto init_type = check_expression(*stmt->initializer);

    // If there's a type annotation, check that initializer matches it
    if (stmt->type_annotation.has_value()) {
        auto declared_type = ast_type_to_semantic(**stmt->type_annotation);

        if (!init_type->equals(*declared_type)) {
            type_mismatch_error(stmt->initializer->location, *declared_type, *init_type);
            // Use declared type for error recovery
            init_type = std::move(declared_type);
        }
    }

    // Type check the pattern and bind variables
    check_pattern(*stmt->pattern, *init_type);
}

auto TypeChecker::visit_return(ast::ReturnStmt* stmt) -> void {
    // Check that we're inside a function
    if (!current_function_return_type_) {
        error(stmt->location, "Return statement outside of function");
        return;
    }

    // Type check the return value
    auto return_type = check_expression(*stmt->value);

    // Check that return type matches function's declared return type
    if (!return_type->equals(*current_function_return_type_)) {
        type_mismatch_error(stmt->value->location,
                           *current_function_return_type_,
                           *return_type);
    }
}

auto TypeChecker::visit_expr_stmt(ast::ExprStmt* stmt) -> void {
    // Just check the expression, ignore the result
    check_expression(*stmt->expression);
}

// ===== Helper Methods =====

auto TypeChecker::error(SourceLocation location, std::string message) -> void {
    result_.add_error(location, std::move(message));
}

auto TypeChecker::type_mismatch_error(SourceLocation location,
                                     const SemanticType& expected,
                                     const SemanticType& actual) -> void {
    error(location, fmt::format("Type mismatch: expected '{}', got '{}'",
                                expected.to_string(), actual.to_string()));
}

auto TypeChecker::ast_type_to_semantic(const ast::Type& ast_type) -> std::unique_ptr<SemanticType> {
    switch (ast_type.kind) {
        case ast::TypeKind::Named: {
            auto& named = static_cast<const ast::NamedType&>(ast_type);
            auto builtin = type_env_.get_builtin(named.name);
            if (builtin) {
                return std::move(*builtin);
            }
            // Unknown type - could be user-defined (not supported yet)
            return std::make_unique<UnknownType>();
        }

        case ast::TypeKind::List: {
            auto& list = static_cast<const ast::ListType&>(ast_type);
            auto elem_type = ast_type_to_semantic(*list.element_type);
            return std::make_unique<ListType>(std::move(elem_type));
        }

        case ast::TypeKind::Tuple: {
            auto& tuple = static_cast<const ast::TupleType&>(ast_type);
            std::vector<std::unique_ptr<SemanticType>> elem_types;
            for (auto& elem : tuple.element_types) {
                elem_types.push_back(ast_type_to_semantic(*elem));
            }
            return std::make_unique<TupleType>(std::move(elem_types));
        }
    }

    return std::make_unique<UnknownType>();
}

// ===== Binary Operator Type Checking =====

auto TypeChecker::check_binary_arithmetic(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType> {
    auto left_type = check_expression(*expr->left);
    auto right_type = check_expression(*expr->right);

    // Both operands must be Int or Float
    bool left_is_numeric = (left_type->kind == TypeKind::Primitive &&
        (static_cast<PrimitiveType*>(left_type.get())->primitive_kind == PrimitiveKind::Int ||
         static_cast<PrimitiveType*>(left_type.get())->primitive_kind == PrimitiveKind::Float));

    bool right_is_numeric = (right_type->kind == TypeKind::Primitive &&
        (static_cast<PrimitiveType*>(right_type.get())->primitive_kind == PrimitiveKind::Int ||
         static_cast<PrimitiveType*>(right_type.get())->primitive_kind == PrimitiveKind::Float));

    if (!left_is_numeric) {
        error(expr->left->location,
              fmt::format("Arithmetic operator requires numeric type, got '{}'",
                         left_type->to_string()));
        return std::make_unique<UnknownType>();
    }

    if (!right_is_numeric) {
        error(expr->right->location,
              fmt::format("Arithmetic operator requires numeric type, got '{}'",
                         right_type->to_string()));
        return std::make_unique<UnknownType>();
    }

    // If both are Int, result is Int
    // If either is Float, result is Float (type promotion)
    auto* left_prim = static_cast<PrimitiveType*>(left_type.get());
    auto* right_prim = static_cast<PrimitiveType*>(right_type.get());

    if (left_prim->primitive_kind == PrimitiveKind::Float ||
        right_prim->primitive_kind == PrimitiveKind::Float) {
        return std::make_unique<PrimitiveType>(PrimitiveKind::Float);
    }

    return std::make_unique<PrimitiveType>(PrimitiveKind::Int);
}

auto TypeChecker::check_binary_comparison(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType> {
    auto left_type = check_expression(*expr->left);
    auto right_type = check_expression(*expr->right);

    using Op = ast::BinaryOp;

    // For ordering comparisons (<, >, <=, >=), both sides must be Int or Float
    if (expr->op == Op::Lt || expr->op == Op::Gt ||
        expr->op == Op::Le || expr->op == Op::Ge) {

        bool left_is_numeric = (left_type->kind == TypeKind::Primitive &&
            (static_cast<PrimitiveType*>(left_type.get())->primitive_kind == PrimitiveKind::Int ||
             static_cast<PrimitiveType*>(left_type.get())->primitive_kind == PrimitiveKind::Float));

        bool right_is_numeric = (right_type->kind == TypeKind::Primitive &&
            (static_cast<PrimitiveType*>(right_type.get())->primitive_kind == PrimitiveKind::Int ||
             static_cast<PrimitiveType*>(right_type.get())->primitive_kind == PrimitiveKind::Float));

        if (!left_is_numeric || !right_is_numeric) {
            error(expr->location,
                  "Ordering comparison requires numeric types");
            return std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
        }
    }

    // For equality comparisons (==, !=), both sides must have same type
    if (expr->op == Op::Eq || expr->op == Op::Ne) {
        if (!left_type->equals(*right_type)) {
            type_mismatch_error(expr->right->location, *left_type, *right_type);
        }
    }

    // All comparisons return Bool
    return std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
}

auto TypeChecker::check_binary_logical(ast::BinaryExpr* expr) -> std::unique_ptr<SemanticType> {
    auto left_type = check_expression(*expr->left);
    auto right_type = check_expression(*expr->right);

    // Both operands must be Bool
    auto bool_type = PrimitiveType(PrimitiveKind::Bool);

    if (!left_type->equals(bool_type)) {
        type_mismatch_error(expr->left->location, bool_type, *left_type);
    }

    if (!right_type->equals(bool_type)) {
        type_mismatch_error(expr->right->location, bool_type, *right_type);
    }

    return std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
}

// ===== Unary Operator Type Checking =====

auto TypeChecker::check_unary_arithmetic(ast::UnaryExpr* expr) -> std::unique_ptr<SemanticType> {
    auto operand_type = check_expression(*expr->operand);

    // Operand must be Int or Float
    bool is_numeric = (operand_type->kind == TypeKind::Primitive &&
        (static_cast<PrimitiveType*>(operand_type.get())->primitive_kind == PrimitiveKind::Int ||
         static_cast<PrimitiveType*>(operand_type.get())->primitive_kind == PrimitiveKind::Float));

    if (!is_numeric) {
        error(expr->operand->location,
              fmt::format("Unary arithmetic operator requires numeric type, got '{}'",
                         operand_type->to_string()));
        return std::make_unique<UnknownType>();
    }

    // Result has same type as operand
    return operand_type;
}

auto TypeChecker::check_unary_logical(ast::UnaryExpr* expr) -> std::unique_ptr<SemanticType> {
    auto operand_type = check_expression(*expr->operand);

    // Operand must be Bool
    auto bool_type = PrimitiveType(PrimitiveKind::Bool);

    if (!operand_type->equals(bool_type)) {
        type_mismatch_error(expr->operand->location, bool_type, *operand_type);
    }

    return std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
}

// ===== Pattern Checking =====

auto TypeChecker::check_pattern(ast::Pattern& pattern, const SemanticType& expected_type) -> void {
    switch (pattern.kind) {
        case ast::PatternKind::Identifier: {
            auto* id_pattern = static_cast<ast::IdentifierPattern*>(&pattern);

            // Declare the identifier with the expected type
            bool success = symbol_table_.declare(
                id_pattern->name,
                SymbolKind::Variable,
                expected_type.clone(),
                pattern.location
            );

            if (!success) {
                error(pattern.location,
                      fmt::format("Variable '{}' is already declared in this scope",
                                 id_pattern->name));
            }
            break;
        }

        case ast::PatternKind::Tuple: {
            auto* tuple_pattern = static_cast<ast::TuplePattern*>(&pattern);

            // Expected type must be a tuple
            if (expected_type.kind != TypeKind::Tuple) {
                error(pattern.location,
                      fmt::format("Cannot destructure non-tuple type '{}' with tuple pattern",
                                 expected_type.to_string()));
                return;
            }

            auto& tuple_type = static_cast<const TupleType&>(expected_type);

            // Check that pattern arity matches tuple arity
            if (tuple_pattern->elements.size() != tuple_type.element_types.size()) {
                error(pattern.location,
                      fmt::format("Tuple pattern has {} elements but type has {} elements",
                                 tuple_pattern->elements.size(),
                                 tuple_type.element_types.size()));
                return;
            }

            // Recursively check each element pattern
            for (size_t i = 0; i < tuple_pattern->elements.size(); ++i) {
                check_pattern(*tuple_pattern->elements[i], *tuple_type.element_types[i]);
            }
            break;
        }
    }
}

} // namespace semantic
} // namespace lucid
