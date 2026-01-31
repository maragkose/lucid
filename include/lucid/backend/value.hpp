#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace lucid::backend {

// Forward declaration
class Value;

// Runtime value types
enum class ValueType {
    Int,
    Float,
    Bool,
    String,
    List,
    Tuple,
    Function,
};

// Runtime value representation
class Value {
public:
    // Constructors
    Value();  // Default: Int(0)
    explicit Value(int64_t value);
    explicit Value(double value);
    explicit Value(bool value);
    explicit Value(std::string value);
    explicit Value(std::vector<Value> elements, bool is_tuple = false);

    // Function value constructor
    static auto make_function(size_t function_index, std::string name) -> Value;

    // Destructor
    ~Value();

    // Copy and move semantics
    Value(const Value& other);
    Value(Value&& other) noexcept;
    auto operator=(const Value& other) -> Value&;
    auto operator=(Value&& other) noexcept -> Value&;

    // Type checking
    auto type() const -> ValueType { return type_; }
    auto is_int() const -> bool { return type_ == ValueType::Int; }
    auto is_float() const -> bool { return type_ == ValueType::Float; }
    auto is_bool() const -> bool { return type_ == ValueType::Bool; }
    auto is_string() const -> bool { return type_ == ValueType::String; }
    auto is_list() const -> bool { return type_ == ValueType::List; }
    auto is_tuple() const -> bool { return type_ == ValueType::Tuple; }
    auto is_function() const -> bool { return type_ == ValueType::Function; }

    // Type conversions (with runtime checking)
    auto as_int() const -> int64_t;
    auto as_float() const -> double;
    auto as_bool() const -> bool;
    auto as_string() const -> const std::string&;
    auto as_list() const -> const std::vector<Value>&;
    auto as_tuple() const -> const std::vector<Value>&;
    auto as_function_index() const -> size_t;
    auto as_function_name() const -> const std::string&;

    // Mutable accessors (for collections)
    auto as_list_mut() -> std::vector<Value>&;
    auto as_tuple_mut() -> std::vector<Value>&;

    // Equality and comparison
    auto operator==(const Value& other) const -> bool;
    auto operator!=(const Value& other) const -> bool { return !(*this == other); }
    auto operator<(const Value& other) const -> bool;
    auto operator>(const Value& other) const -> bool { return other < *this; }
    auto operator<=(const Value& other) const -> bool { return !(other < *this); }
    auto operator>=(const Value& other) const -> bool { return !(*this < other); }

    // Truthiness (for logical operations)
    auto is_truthy() const -> bool;

    // String representation (for debugging)
    auto to_string() const -> std::string;
    auto type_name() const -> std::string_view;

private:
    ValueType type_;

    // Tagged union for value storage
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        std::string* string_val;              // Heap allocated
        std::vector<Value>* list_val;         // Heap allocated
        std::vector<Value>* tuple_val;        // Heap allocated
        struct {
            size_t index;
            std::string* name;
        } function_val;                       // Function reference
    };

    // Helper methods
    auto clear() -> void;  // Clean up heap-allocated data
    auto copy_from(const Value& other) -> void;
};

} // namespace lucid::backend
