#include <lucid/backend/compiler.hpp>
#include <fmt/format.h>
#include <stdexcept>

namespace lucid::backend {

// Constructor
Compiler::Compiler() = default;

// Main compilation entry point
auto Compiler::compile(ast::Program* program) -> Bytecode {
    // Pass 1: Collect all functions
    collect_functions(program);

    // Pass 2: Compile each function
    for (auto& function : program->functions) {
        compile_function(function.get());
    }

    // Emit HALT at the end
    emit(OpCode::HALT);

    return std::move(bytecode_);
}

// ===== Two-Pass Compilation =====

// Pass 1: Collect function signatures
auto Compiler::collect_functions(ast::Program* program) -> void {
    for (auto& function : program->functions) {
        size_t offset = bytecode_.current_offset();  // Will be updated in Pass 2
        size_t param_count = function->parameters.size();

        // Add function to bytecode's function table
        size_t func_idx = bytecode_.add_function(
            function->name,
            offset,  // Placeholder offset
            param_count,
            param_count  // local_count will be updated in Pass 2
        );

        // Store function index for name resolution
        function_indices_[function->name] = func_idx;
    }
}

// Pass 2: Compile function body
auto Compiler::compile_function(ast::FunctionDef* function) -> void {
    // Update function offset to current position
    size_t func_idx = function_indices_[function->name];
    bytecode_.functions[func_idx].offset = bytecode_.current_offset();

    // Create function context
    FunctionContext func_ctx{function->name, function->parameters.size(), bytecode_.current_offset()};
    current_function_ = &func_ctx;

    // Enter function scope
    enter_scope();

    // Declare parameters as local variables
    for (const auto& param : function->parameters) {
        declare_local(param->name);
    }

    // Compile function body
    function->body->accept(*this);

    // If body doesn't end with explicit RETURN, add implicit return
    // (This handles functions that end with expression-only blocks)
    // Note: Type checker ensures function returns correct type
    if (bytecode_.instructions.empty() ||
        bytecode_.instructions.back() != static_cast<uint8_t>(OpCode::RETURN)) {
        emit(OpCode::RETURN);
    }

    // Update local count
    bytecode_.functions[func_idx].local_count = scopes_.back().local_count;

    // Exit function scope
    exit_scope();
    current_function_ = nullptr;
}

// ===== Scope Management =====

auto Compiler::enter_scope() -> void {
    scopes_.push_back(Scope{});
}

auto Compiler::exit_scope() -> void {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

auto Compiler::declare_local(const std::string& name) -> size_t {
    if (scopes_.empty()) {
        throw std::runtime_error("Cannot declare local outside of scope");
    }

    auto& scope = scopes_.back();
    size_t index = scope.local_count++;
    scope.locals[name] = index;
    return index;
}

auto Compiler::resolve_local(const std::string& name) -> int {
    // Search scopes from innermost to outermost
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->locals.find(name);
        if (found != it->locals.end()) {
            return static_cast<int>(found->second);
        }
    }
    return -1;  // Not found
}

auto Compiler::resolve_function(const std::string& name) -> int {
    auto found = function_indices_.find(name);
    if (found != function_indices_.end()) {
        return static_cast<int>(found->second);
    }
    return -1;  // Not found
}

// ===== Bytecode Emission Helpers =====

auto Compiler::emit(OpCode opcode) -> void {
    bytecode_.emit(opcode);
}

auto Compiler::emit(OpCode opcode, uint8_t operand) -> void {
    bytecode_.emit(opcode, operand);
}

auto Compiler::emit(OpCode opcode, uint16_t operand) -> void {
    bytecode_.emit(opcode, operand);
}

auto Compiler::emit(OpCode opcode, uint16_t operand1, uint8_t operand2) -> void {
    bytecode_.emit(opcode, operand1, operand2);
}

auto Compiler::add_constant(Value value) -> uint16_t {
    return bytecode_.add_constant(std::move(value));
}

auto Compiler::emit_jump(OpCode opcode) -> size_t {
    emit(opcode, uint16_t(0xFFFF));  // Placeholder offset
    return bytecode_.current_offset() - 3;  // Return offset of jump instruction
}

auto Compiler::patch_jump(size_t offset) -> void {
    // Calculate jump offset from after the jump instruction
    int16_t jump = static_cast<int16_t>(bytecode_.current_offset() - (offset + 3));
    bytecode_.patch_jump(offset, jump);
}

// ===== Expression Visitors =====

auto Compiler::visit_int_literal(ast::IntLiteralExpr* expr) -> void {
    uint16_t const_idx = add_constant(Value(expr->value));
    emit(OpCode::CONSTANT, const_idx);
}

auto Compiler::visit_float_literal(ast::FloatLiteralExpr* expr) -> void {
    uint16_t const_idx = add_constant(Value(expr->value));
    emit(OpCode::CONSTANT, const_idx);
}

auto Compiler::visit_string_literal(ast::StringLiteralExpr* expr) -> void {
    uint16_t const_idx = add_constant(Value(expr->value));
    emit(OpCode::CONSTANT, const_idx);
}

auto Compiler::visit_bool_literal(ast::BoolLiteralExpr* expr) -> void {
    if (expr->value) {
        emit(OpCode::TRUE);
    } else {
        emit(OpCode::FALSE);
    }
}

auto Compiler::visit_identifier(ast::IdentifierExpr* expr) -> void {
    // Try to resolve as local variable
    int local_idx = resolve_local(expr->name);
    if (local_idx >= 0) {
        emit(OpCode::LOAD_LOCAL, static_cast<uint16_t>(local_idx));
        return;
    }

    // Try to resolve as function
    int func_idx = resolve_function(expr->name);
    if (func_idx >= 0) {
        emit(OpCode::LOAD_GLOBAL, static_cast<uint16_t>(func_idx));
        return;
    }

    // Not found - this should not happen after type checking
    throw std::runtime_error(fmt::format("Undefined identifier: {}", expr->name));
}

auto Compiler::visit_binary(ast::BinaryExpr* expr) -> void {
    // Compile operands (left then right for stack-based evaluation)
    expr->left->accept(*this);
    expr->right->accept(*this);

    // Emit operator
    switch (expr->op) {
        case ast::BinaryOp::Add: emit(OpCode::ADD); break;
        case ast::BinaryOp::Sub: emit(OpCode::SUB); break;
        case ast::BinaryOp::Mul: emit(OpCode::MUL); break;
        case ast::BinaryOp::Div: emit(OpCode::DIV); break;
        case ast::BinaryOp::Mod: emit(OpCode::MOD); break;
        case ast::BinaryOp::Pow: emit(OpCode::POW); break;
        case ast::BinaryOp::Eq:  emit(OpCode::EQ); break;
        case ast::BinaryOp::Ne:  emit(OpCode::NE); break;
        case ast::BinaryOp::Lt:  emit(OpCode::LT); break;
        case ast::BinaryOp::Gt:  emit(OpCode::GT); break;
        case ast::BinaryOp::Le:  emit(OpCode::LE); break;
        case ast::BinaryOp::Ge:  emit(OpCode::GE); break;
        case ast::BinaryOp::And: emit(OpCode::AND); break;
        case ast::BinaryOp::Or:  emit(OpCode::OR); break;
    }
}

auto Compiler::visit_unary(ast::UnaryExpr* expr) -> void {
    // Compile operand
    expr->operand->accept(*this);

    // Emit operator
    switch (expr->op) {
        case ast::UnaryOp::Not: emit(OpCode::NOT); break;
        case ast::UnaryOp::Neg: emit(OpCode::NEGATE); break;
        case ast::UnaryOp::Pos: emit(OpCode::POSITIVE); break;
    }
}

auto Compiler::visit_tuple(ast::TupleExpr* expr) -> void {
    // Compile each element
    for (const auto& element : expr->elements) {
        element->accept(*this);
    }

    // Build tuple from stack
    emit(OpCode::BUILD_TUPLE, static_cast<uint16_t>(expr->elements.size()));
}

auto Compiler::visit_list(ast::ListExpr* expr) -> void {
    // Compile each element
    for (const auto& element : expr->elements) {
        element->accept(*this);
    }

    // Build list from stack
    emit(OpCode::BUILD_LIST, static_cast<uint16_t>(expr->elements.size()));
}

auto Compiler::visit_index(ast::IndexExpr* expr) -> void {
    // Compile object and index
    expr->object->accept(*this);
    expr->index->accept(*this);

    // Emit index operation
    emit(OpCode::INDEX);
}

auto Compiler::visit_call(ast::CallExpr* expr) -> void {
    // Check for built-in functions first
    if (auto* ident = dynamic_cast<ast::IdentifierExpr*>(expr->callee.get())) {
        // Check for builtin functions
        if (ident->name == "print") {
            // Compile arguments
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::PRINT),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "println") {
            // Compile arguments
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::PRINTLN),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "to_string") {
            // Compile arguments
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::TO_STRING),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "read_file") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::READ_FILE),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "write_file") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::WRITE_FILE),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "append_file") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::APPEND_FILE),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
        if (ident->name == "file_exists") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emit(OpCode::CALL_BUILTIN, static_cast<uint16_t>(BuiltinId::FILE_EXISTS),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
    }

    // Compile arguments (left to right)
    for (const auto& arg : expr->arguments) {
        arg->accept(*this);
    }

    // Compile callee to get function reference
    expr->callee->accept(*this);

    // For now, we expect callee to be an identifier
    // Extract function index if it's a direct call
    if (auto* ident = dynamic_cast<ast::IdentifierExpr*>(expr->callee.get())) {
        int func_idx = resolve_function(ident->name);
        if (func_idx >= 0) {
            // Pop the LOAD_GLOBAL we just emitted
            bytecode_.instructions.pop_back();
            bytecode_.instructions.pop_back();
            bytecode_.instructions.pop_back();

            // Emit direct call
            emit(OpCode::CALL, static_cast<uint16_t>(func_idx),
                 static_cast<uint8_t>(expr->arguments.size()));
            return;
        }
    }

    throw std::runtime_error("Only direct function calls supported in Day 2");
}

auto Compiler::visit_method_call(ast::MethodCallExpr* expr) -> void {
    // Compile object
    expr->object->accept(*this);

    // Compile arguments
    for (const auto& arg : expr->arguments) {
        arg->accept(*this);
    }

    // Add method name to constants
    uint16_t name_idx = add_constant(Value(expr->method_name));

    // Emit method call
    emit(OpCode::CALL_METHOD, name_idx, static_cast<uint8_t>(expr->arguments.size()));
}

auto Compiler::visit_if(ast::IfExpr* expr) -> void {
    // Compile condition
    expr->condition->accept(*this);

    // Jump to else if condition is false
    size_t else_jump = emit_jump(OpCode::JUMP_IF_FALSE);

    // Pop condition result
    emit(OpCode::POP);

    // Compile then branch
    expr->then_branch->accept(*this);

    // Jump over else branch
    size_t end_jump = emit_jump(OpCode::JUMP);

    // Patch else jump
    patch_jump(else_jump);

    // Pop condition result (for else path)
    emit(OpCode::POP);

    // Compile else branch if present
    if (expr->else_branch.has_value()) {
        expr->else_branch.value()->accept(*this);
    } else {
        // If no else, push false (or could be unit)
        emit(OpCode::FALSE);
    }

    // Patch end jump
    patch_jump(end_jump);
}

auto Compiler::visit_block(ast::BlockExpr* expr) -> void {
    // Empty block - push unit/false
    if (expr->statements.empty()) {
        emit(OpCode::FALSE);
        return;
    }

    // DEBUG: Print number of statements
    // fmt::print("Block has {} statements\n", expr->statements.size());

    // Compile all statements except the last
    for (size_t i = 0; i < expr->statements.size() - 1; ++i) {
        expr->statements[i]->accept(*this);
    }

    // Handle last statement specially
    auto& last_stmt = expr->statements.back();

    // If last statement is an ExprStmt, compile the expression directly
    // without the POP (to leave the value on stack as block result)
    if (auto* expr_stmt = dynamic_cast<ast::ExprStmt*>(last_stmt.get())) {
        expr_stmt->expression->accept(*this);
    } else {
        // Other statements (let, return, etc.) compile normally
        last_stmt->accept(*this);
    }

    // Block value is the value left on the stack by last statement
}

auto Compiler::visit_lambda(ast::LambdaExpr* expr) -> void {
    (void)expr;  // Unused in Day 2
    // Lambdas not implemented in Day 2
    throw std::runtime_error("Lambda expressions not yet implemented");
}

// ===== Statement Visitors =====

auto Compiler::visit_let(ast::LetStmt* stmt) -> void {
    // Compile initializer
    stmt->initializer->accept(*this);

    // Compile pattern (handles variable declaration)
    compile_pattern(stmt->pattern.get(), true);
}

auto Compiler::visit_return(ast::ReturnStmt* stmt) -> void {
    // Compile return value
    stmt->value->accept(*this);

    // Emit return instruction
    emit(OpCode::RETURN);
}

auto Compiler::visit_expr_stmt(ast::ExprStmt* stmt) -> void {
    // Compile expression
    stmt->expression->accept(*this);

    // Pop result (expression statement doesn't leave value on stack)
    emit(OpCode::POP);
}

// ===== Pattern Compilation =====

auto Compiler::compile_pattern(ast::Pattern* pattern, bool is_declaration) -> void {
    switch (pattern->kind) {
        case ast::PatternKind::Identifier: {
            auto* id_pattern = static_cast<ast::IdentifierPattern*>(pattern);

            if (is_declaration) {
                // Declare new local variable
                size_t local_idx = declare_local(id_pattern->name);
                emit(OpCode::STORE_LOCAL, static_cast<uint16_t>(local_idx));
            } else {
                // Store to existing variable
                int local_idx = resolve_local(id_pattern->name);
                if (local_idx >= 0) {
                    emit(OpCode::STORE_LOCAL, static_cast<uint16_t>(local_idx));
                } else {
                    throw std::runtime_error(fmt::format("Undefined variable: {}", id_pattern->name));
                }
            }
            break;
        }

        case ast::PatternKind::Tuple: {
            auto* tuple_pattern = static_cast<ast::TuplePattern*>(pattern);

            // Tuple is on stack, we need to destructure it
            // For each element, index into tuple and store
            for (size_t i = 0; i < tuple_pattern->elements.size(); ++i) {
                // Duplicate tuple on stack
                emit(OpCode::DUP);

                // Push index
                uint16_t idx_const = add_constant(Value(static_cast<int64_t>(i)));
                emit(OpCode::CONSTANT, idx_const);

                // Index into tuple
                emit(OpCode::INDEX);

                // Recursively compile pattern
                compile_pattern(tuple_pattern->elements[i].get(), is_declaration);
            }

            // Pop original tuple
            emit(OpCode::POP);
            break;
        }
    }
}

} // namespace lucid::backend
