#include <lucid/semantic/type_system.hpp>
#include <sstream>

namespace lucid {
namespace semantic {

// ===== PrimitiveType Implementation =====

auto PrimitiveType::primitive_name(PrimitiveKind pk) -> std::string {
    switch (pk) {
        case PrimitiveKind::Int: return "Int";
        case PrimitiveKind::Float: return "Float";
        case PrimitiveKind::String: return "String";
        case PrimitiveKind::Bool: return "Bool";
    }
    return "Unknown";
}

auto PrimitiveType::equals(const SemanticType& other) const -> bool {
    if (other.kind != TypeKind::Primitive) return false;
    const auto& other_prim = static_cast<const PrimitiveType&>(other);
    return primitive_kind == other_prim.primitive_kind;
}

auto PrimitiveType::to_string() const -> std::string {
    return primitive_name(primitive_kind);
}

auto PrimitiveType::clone() const -> std::unique_ptr<SemanticType> {
    return std::make_unique<PrimitiveType>(primitive_kind);
}

auto PrimitiveType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_primitive(*this);
}

// ===== ListType Implementation =====

auto ListType::equals(const SemanticType& other) const -> bool {
    if (other.kind != TypeKind::List) return false;
    const auto& other_list = static_cast<const ListType&>(other);
    return element_type->equals(*other_list.element_type);
}

auto ListType::to_string() const -> std::string {
    return "List[" + element_type->to_string() + "]";
}

auto ListType::clone() const -> std::unique_ptr<SemanticType> {
    return std::make_unique<ListType>(element_type->clone());
}

auto ListType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_list(*this);
}

// ===== TupleType Implementation =====

auto TupleType::equals(const SemanticType& other) const -> bool {
    if (other.kind != TypeKind::Tuple) return false;
    const auto& other_tuple = static_cast<const TupleType&>(other);

    if (element_types.size() != other_tuple.element_types.size()) {
        return false;
    }

    for (size_t i = 0; i < element_types.size(); ++i) {
        if (!element_types[i]->equals(*other_tuple.element_types[i])) {
            return false;
        }
    }

    return true;
}

auto TupleType::to_string() const -> std::string {
    if (element_types.empty()) {
        return "()";
    }

    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < element_types.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << element_types[i]->to_string();
    }
    oss << ")";
    return oss.str();
}

auto TupleType::clone() const -> std::unique_ptr<SemanticType> {
    std::vector<std::unique_ptr<SemanticType>> cloned_elems;
    cloned_elems.reserve(element_types.size());
    for (const auto& elem : element_types) {
        cloned_elems.push_back(elem->clone());
    }
    return std::make_unique<TupleType>(std::move(cloned_elems));
}

auto TupleType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_tuple(*this);
}

// ===== FunctionType Implementation =====

auto FunctionType::equals(const SemanticType& other) const -> bool {
    if (other.kind != TypeKind::Function) return false;
    const auto& other_func = static_cast<const FunctionType&>(other);

    if (param_types.size() != other_func.param_types.size()) {
        return false;
    }

    for (size_t i = 0; i < param_types.size(); ++i) {
        if (!param_types[i]->equals(*other_func.param_types[i])) {
            return false;
        }
    }

    return return_type->equals(*other_func.return_type);
}

auto FunctionType::to_string() const -> std::string {
    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < param_types.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << param_types[i]->to_string();
    }
    oss << ") -> " << return_type->to_string();
    return oss.str();
}

auto FunctionType::clone() const -> std::unique_ptr<SemanticType> {
    std::vector<std::unique_ptr<SemanticType>> cloned_params;
    cloned_params.reserve(param_types.size());
    for (const auto& param : param_types) {
        cloned_params.push_back(param->clone());
    }
    return std::make_unique<FunctionType>(
        std::move(cloned_params),
        return_type->clone()
    );
}

auto FunctionType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_function(*this);
}

// ===== TypeVariable Implementation =====

auto TypeVariable::equals(const SemanticType& other) const -> bool {
    if (other.kind != TypeKind::TypeVariable) return false;
    const auto& other_var = static_cast<const TypeVariable&>(other);
    return name == other_var.name;
}

auto TypeVariable::to_string() const -> std::string {
    return name;
}

auto TypeVariable::clone() const -> std::unique_ptr<SemanticType> {
    return std::make_unique<TypeVariable>(name);
}

auto TypeVariable::accept(TypeVisitor& visitor) -> void {
    visitor.visit_type_variable(*this);
}

// ===== UnknownType Implementation =====

auto UnknownType::equals(const SemanticType& /* other */) const -> bool {
    // Unknown types are never equal to anything (including other Unknown types)
    return false;
}

auto UnknownType::to_string() const -> std::string {
    return "?";
}

auto UnknownType::clone() const -> std::unique_ptr<SemanticType> {
    return std::make_unique<UnknownType>();
}

auto UnknownType::accept(TypeVisitor& visitor) -> void {
    visitor.visit_unknown(*this);
}

// ===== Type Operations =====

auto types_compatible(const SemanticType& t1, const SemanticType& t2) -> bool {
    // For now, types are compatible if they're equal
    // In the future, this could handle subtyping, coercion, etc.
    return t1.equals(t2);
}

auto unify_types(const SemanticType& t1, const SemanticType& t2)
    -> std::optional<std::unique_ptr<SemanticType>> {

    // Simple unification: if types are equal, return a clone of t1
    if (t1.equals(t2)) {
        return t1.clone();
    }

    // Type variables can unify with anything
    if (t1.kind == TypeKind::TypeVariable) {
        return t2.clone();
    }
    if (t2.kind == TypeKind::TypeVariable) {
        return t1.clone();
    }

    // Unknown types unify with anything
    if (t1.kind == TypeKind::Unknown || t2.kind == TypeKind::Unknown) {
        return std::make_unique<UnknownType>();
    }

    // No unification possible
    return std::nullopt;
}

// ===== TypeEnvironment Implementation =====

TypeEnvironment::TypeEnvironment() {
    builtin_names_ = {"Int", "Float", "String", "Bool"};
}

auto TypeEnvironment::get_builtin(const std::string& name)
    -> std::optional<std::unique_ptr<SemanticType>> {

    if (name == "Int") {
        return std::make_unique<PrimitiveType>(PrimitiveKind::Int);
    }
    if (name == "Float") {
        return std::make_unique<PrimitiveType>(PrimitiveKind::Float);
    }
    if (name == "String") {
        return std::make_unique<PrimitiveType>(PrimitiveKind::String);
    }
    if (name == "Bool") {
        return std::make_unique<PrimitiveType>(PrimitiveKind::Bool);
    }

    return std::nullopt;
}

auto TypeEnvironment::is_builtin(const std::string& name) const -> bool {
    for (const auto& builtin : builtin_names_) {
        if (builtin == name) return true;
    }
    return false;
}

} // namespace semantic
} // namespace lucid
