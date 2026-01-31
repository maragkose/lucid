#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace lucid {
namespace semantic {

// Forward declarations
class TypeVisitor;

// ===== Type Kind Enumeration =====

enum class TypeKind {
    Primitive,    // Int, Float, String, Bool
    List,         // List[T]
    Tuple,        // (T1, T2, ...)
    Function,     // (T1, T2, ...) -> R
    TypeVariable, // For type inference (e.g., 'a, 'b)
    Unknown       // Error recovery
};

// ===== Base Type Class =====

class SemanticType {
public:
    TypeKind kind;

    explicit SemanticType(TypeKind kind) : kind(kind) {}
    virtual ~SemanticType() = default;

    // Delete copy, allow move
    SemanticType(const SemanticType&) = delete;
    SemanticType& operator=(const SemanticType&) = delete;
    SemanticType(SemanticType&&) = default;
    SemanticType& operator=(SemanticType&&) = default;

    // Type operations
    virtual auto equals(const SemanticType& other) const -> bool = 0;
    virtual auto to_string() const -> std::string = 0;
    virtual auto clone() const -> std::unique_ptr<SemanticType> = 0;

    // Visitor pattern
    virtual auto accept(TypeVisitor& visitor) -> void = 0;
};

// ===== Primitive Type =====

enum class PrimitiveKind {
    Int,
    Float,
    String,
    Bool
};

class PrimitiveType : public SemanticType {
public:
    PrimitiveKind primitive_kind;

    explicit PrimitiveType(PrimitiveKind pk)
        : SemanticType(TypeKind::Primitive), primitive_kind(pk) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;

    // Helper to get primitive type name
    static auto primitive_name(PrimitiveKind pk) -> std::string;
};

// ===== List Type =====

class ListType : public SemanticType {
public:
    std::unique_ptr<SemanticType> element_type;

    explicit ListType(std::unique_ptr<SemanticType> elem)
        : SemanticType(TypeKind::List), element_type(std::move(elem)) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Tuple Type =====

class TupleType : public SemanticType {
public:
    std::vector<std::unique_ptr<SemanticType>> element_types;

    explicit TupleType(std::vector<std::unique_ptr<SemanticType>> elems)
        : SemanticType(TypeKind::Tuple), element_types(std::move(elems)) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Function Type =====

class FunctionType : public SemanticType {
public:
    std::vector<std::unique_ptr<SemanticType>> param_types;
    std::unique_ptr<SemanticType> return_type;

    FunctionType(std::vector<std::unique_ptr<SemanticType>> params,
                 std::unique_ptr<SemanticType> ret)
        : SemanticType(TypeKind::Function),
          param_types(std::move(params)),
          return_type(std::move(ret)) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Type Variable (for type inference) =====

class TypeVariable : public SemanticType {
public:
    std::string name;  // e.g., "'a", "'b"

    explicit TypeVariable(std::string n)
        : SemanticType(TypeKind::TypeVariable), name(std::move(n)) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Unknown Type (for error recovery) =====

class UnknownType : public SemanticType {
public:
    UnknownType() : SemanticType(TypeKind::Unknown) {}

    auto equals(const SemanticType& other) const -> bool override;
    auto to_string() const -> std::string override;
    auto clone() const -> std::unique_ptr<SemanticType> override;
    auto accept(TypeVisitor& visitor) -> void override;
};

// ===== Type Visitor =====

class TypeVisitor {
public:
    virtual ~TypeVisitor() = default;

    virtual auto visit_primitive(PrimitiveType& type) -> void = 0;
    virtual auto visit_list(ListType& type) -> void = 0;
    virtual auto visit_tuple(TupleType& type) -> void = 0;
    virtual auto visit_function(FunctionType& type) -> void = 0;
    virtual auto visit_type_variable(TypeVariable& type) -> void = 0;
    virtual auto visit_unknown(UnknownType& type) -> void = 0;
};

// ===== Type Operations =====

// Check if two types are compatible (can be unified)
auto types_compatible(const SemanticType& t1, const SemanticType& t2) -> bool;

// Try to unify two types (for type inference)
auto unify_types(const SemanticType& t1, const SemanticType& t2)
    -> std::optional<std::unique_ptr<SemanticType>>;

// ===== Type Environment =====

class TypeEnvironment {
public:
    TypeEnvironment();

    // Get builtin type by name
    auto get_builtin(const std::string& name) -> std::optional<std::unique_ptr<SemanticType>>;

    // Check if a name is a builtin type
    auto is_builtin(const std::string& name) const -> bool;

private:
    // Store builtin type names (Int, Float, String, Bool)
    std::vector<std::string> builtin_names_;
};

} // namespace semantic
} // namespace lucid
