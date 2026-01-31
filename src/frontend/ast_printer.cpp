#include <lucid/frontend/ast_printer.hpp>
#include <fmt/format.h>

namespace lucid {

// ===== Main Entry Points =====

auto ASTPrinter::print_program(const ast::Program& program) -> std::string {
    output_.str("");
    output_.clear();
    indent_level_ = 0;

    writeln("Program:");
    indent_level_++;

    for (const auto& func : program.functions) {
        indent();
        writeln(fmt::format("Function: {}", func->name));
        indent_level_++;

        // Parameters
        if (!func->parameters.empty()) {
            indent();
            writeln("Parameters:");
            indent_level_++;
            for (const auto& param : func->parameters) {
                indent();
                write(fmt::format("{}: ", param->name));
                param->type->accept(*this);
                output_ << "\n";
            }
            indent_level_--;
        }

        // Return type
        indent();
        write("Returns: ");
        func->return_type->accept(*this);
        output_ << "\n";

        // Body
        indent();
        writeln("Body:");
        indent_level_++;
        func->body->accept(*this);
        indent_level_--;

        indent_level_--;
    }

    return output_.str();
}

auto ASTPrinter::print_expr(ast::Expr* expr) -> std::string {
    output_.str("");
    output_.clear();
    indent_level_ = 0;
    expr->accept(*this);
    return output_.str();
}

auto ASTPrinter::print_stmt(ast::Stmt* stmt) -> std::string {
    output_.str("");
    output_.clear();
    indent_level_ = 0;
    stmt->accept(*this);
    return output_.str();
}

auto ASTPrinter::print_pattern(ast::Pattern* pattern) -> std::string {
    output_.str("");
    output_.clear();
    indent_level_ = 0;
    pattern->accept(*this);
    return output_.str();
}

auto ASTPrinter::print_type(ast::Type* type) -> std::string {
    output_.str("");
    output_.clear();
    indent_level_ = 0;
    type->accept(*this);
    return output_.str();
}

// ===== Expression Visitors =====

auto ASTPrinter::visit_int_literal(ast::IntLiteralExpr* expr) -> void {
    write(fmt::format("IntLiteral({})", expr->value));
}

auto ASTPrinter::visit_float_literal(ast::FloatLiteralExpr* expr) -> void {
    write(fmt::format("FloatLiteral({})", expr->value));
}

auto ASTPrinter::visit_string_literal(ast::StringLiteralExpr* expr) -> void {
    write(fmt::format("StringLiteral(\"{}\")", expr->value));
}

auto ASTPrinter::visit_bool_literal(ast::BoolLiteralExpr* expr) -> void {
    write(fmt::format("BoolLiteral({})", expr->value ? "true" : "false"));
}

auto ASTPrinter::visit_identifier(ast::IdentifierExpr* expr) -> void {
    write(fmt::format("Identifier({})", expr->name));
}

auto ASTPrinter::visit_tuple(ast::TupleExpr* expr) -> void {
    if (expr->elements.empty()) {
        write("Tuple()");
        return;
    }

    writeln("Tuple:");
    indent_level_++;
    for (const auto& elem : expr->elements) {
        indent();
        elem->accept(*this);
        output_ << "\n";
    }
    indent_level_--;
}

auto ASTPrinter::visit_list(ast::ListExpr* expr) -> void {
    if (expr->elements.empty()) {
        write("List[]");
        return;
    }

    writeln("List:");
    indent_level_++;
    for (const auto& elem : expr->elements) {
        indent();
        elem->accept(*this);
        output_ << "\n";
    }
    indent_level_--;
}

auto ASTPrinter::visit_binary(ast::BinaryExpr* expr) -> void {
    writeln(fmt::format("Binary({})", ast::binary_op_name(expr->op)));
    indent_level_++;

    indent();
    write("left: ");
    expr->left->accept(*this);
    output_ << "\n";

    indent();
    write("right: ");
    expr->right->accept(*this);
    output_ << "\n";

    indent_level_--;
}

auto ASTPrinter::visit_unary(ast::UnaryExpr* expr) -> void {
    writeln(fmt::format("Unary({})", ast::unary_op_name(expr->op)));
    indent_level_++;
    indent();
    expr->operand->accept(*this);
    output_ << "\n";
    indent_level_--;
}

auto ASTPrinter::visit_call(ast::CallExpr* expr) -> void {
    writeln("Call:");
    indent_level_++;

    indent();
    write("callee: ");
    expr->callee->accept(*this);
    output_ << "\n";

    if (!expr->arguments.empty()) {
        indent();
        writeln("arguments:");
        indent_level_++;
        for (const auto& arg : expr->arguments) {
            indent();
            arg->accept(*this);
            output_ << "\n";
        }
        indent_level_--;
    }

    indent_level_--;
}

auto ASTPrinter::visit_method_call(ast::MethodCallExpr* expr) -> void {
    writeln(fmt::format("MethodCall({})", expr->method_name));
    indent_level_++;

    indent();
    write("object: ");
    expr->object->accept(*this);
    output_ << "\n";

    if (!expr->arguments.empty()) {
        indent();
        writeln("arguments:");
        indent_level_++;
        for (const auto& arg : expr->arguments) {
            indent();
            arg->accept(*this);
            output_ << "\n";
        }
        indent_level_--;
    }

    indent_level_--;
}

auto ASTPrinter::visit_index(ast::IndexExpr* expr) -> void {
    writeln("Index:");
    indent_level_++;

    indent();
    write("object: ");
    expr->object->accept(*this);
    output_ << "\n";

    indent();
    write("index: ");
    expr->index->accept(*this);
    output_ << "\n";

    indent_level_--;
}

auto ASTPrinter::visit_lambda(ast::LambdaExpr* expr) -> void {
    write("Lambda(");
    for (size_t i = 0; i < expr->parameters.size(); ++i) {
        if (i > 0) write(", ");
        write(expr->parameters[i]);
    }
    writeln(")");

    indent_level_++;
    indent();
    write("body: ");
    expr->body->accept(*this);
    output_ << "\n";
    indent_level_--;
}

auto ASTPrinter::visit_if(ast::IfExpr* expr) -> void {
    writeln("If:");
    indent_level_++;

    indent();
    write("condition: ");
    expr->condition->accept(*this);
    output_ << "\n";

    indent();
    writeln("then:");
    indent_level_++;
    indent();
    expr->then_branch->accept(*this);
    output_ << "\n";
    indent_level_--;

    if (expr->else_branch.has_value()) {
        indent();
        writeln("else:");
        indent_level_++;
        indent();
        (*expr->else_branch)->accept(*this);
        output_ << "\n";
        indent_level_--;
    }

    indent_level_--;
}

auto ASTPrinter::visit_block(ast::BlockExpr* expr) -> void {
    if (expr->statements.empty()) {
        write("Block{}");
        return;
    }

    writeln("Block:");
    indent_level_++;
    for (const auto& stmt : expr->statements) {
        indent();
        stmt->accept(*this);
        output_ << "\n";
    }
    indent_level_--;
}

// ===== Statement Visitors =====

auto ASTPrinter::visit_let(ast::LetStmt* stmt) -> void {
    write("Let ");
    stmt->pattern->accept(*this);

    if (stmt->type_annotation.has_value()) {
        write(": ");
        (*stmt->type_annotation)->accept(*this);
    }

    write(" = ");
    stmt->initializer->accept(*this);
}

auto ASTPrinter::visit_return(ast::ReturnStmt* stmt) -> void {
    write("Return ");
    stmt->value->accept(*this);
}

auto ASTPrinter::visit_expr_stmt(ast::ExprStmt* stmt) -> void {
    write("ExprStmt: ");
    stmt->expression->accept(*this);
}

// ===== Pattern Visitors =====

auto ASTPrinter::visit_identifier_pattern(ast::IdentifierPattern* pattern) -> void {
    write(pattern->name);
}

auto ASTPrinter::visit_tuple_pattern(ast::TuplePattern* pattern) -> void {
    write("(");
    for (size_t i = 0; i < pattern->elements.size(); ++i) {
        if (i > 0) write(", ");
        pattern->elements[i]->accept(*this);
    }
    write(")");
}

// ===== Type Visitors =====

auto ASTPrinter::visit_named_type(ast::NamedType* type) -> void {
    write(type->name);
}

auto ASTPrinter::visit_list_type(ast::ListType* type) -> void {
    write("List[");
    type->element_type->accept(*this);
    write("]");
}

auto ASTPrinter::visit_tuple_type(ast::TupleType* type) -> void {
    write("(");
    for (size_t i = 0; i < type->element_types.size(); ++i) {
        if (i > 0) write(", ");
        type->element_types[i]->accept(*this);
    }
    write(")");
}

// ===== Helper Methods =====

auto ASTPrinter::indent() -> void {
    for (int i = 0; i < indent_level_ * INDENT_SIZE; ++i) {
        output_ << ' ';
    }
}

auto ASTPrinter::write(std::string_view text) -> void {
    output_ << text;
}

auto ASTPrinter::writeln(std::string_view text) -> void {
    output_ << text << '\n';
}

} // namespace lucid
