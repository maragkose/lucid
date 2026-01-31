#include <lucid/backend/value.hpp>
#include <fmt/format.h>
#include <stdexcept>
#include <algorithm>

namespace lucid::backend {

// Default constructor
Value::Value() : type_(ValueType::Int), int_val(0) {}

// Int constructor
Value::Value(int64_t value) : type_(ValueType::Int), int_val(value) {}

// Float constructor
Value::Value(double value) : type_(ValueType::Float), float_val(value) {}

// Bool constructor
Value::Value(bool value) : type_(ValueType::Bool), bool_val(value) {}

// String constructor
Value::Value(std::string value) : type_(ValueType::String) {
    string_val = new std::string(std::move(value));
}

// List/Tuple constructor
Value::Value(std::vector<Value> elements, bool is_tuple) {
    if (is_tuple) {
        type_ = ValueType::Tuple;
        tuple_val = new std::vector<Value>(std::move(elements));
    } else {
        type_ = ValueType::List;
        list_val = new std::vector<Value>(std::move(elements));
    }
}

// Function value constructor
auto Value::make_function(size_t function_index, std::string name) -> Value {
    Value val;
    val.type_ = ValueType::Function;
    val.function_val.index = function_index;
    val.function_val.name = new std::string(std::move(name));
    return val;
}

// Destructor
Value::~Value() {
    clear();
}

// Copy constructor
Value::Value(const Value& other) : type_(other.type_) {
    copy_from(other);
}

// Move constructor
Value::Value(Value&& other) noexcept : type_(other.type_) {
    // Move the union data
    switch (type_) {
        case ValueType::Int:
            int_val = other.int_val;
            break;
        case ValueType::Float:
            float_val = other.float_val;
            break;
        case ValueType::Bool:
            bool_val = other.bool_val;
            break;
        case ValueType::String:
            string_val = other.string_val;
            other.string_val = nullptr;
            break;
        case ValueType::List:
            list_val = other.list_val;
            other.list_val = nullptr;
            break;
        case ValueType::Tuple:
            tuple_val = other.tuple_val;
            other.tuple_val = nullptr;
            break;
        case ValueType::Function:
            function_val = other.function_val;
            other.function_val.name = nullptr;
            break;
    }
}

// Copy assignment
auto Value::operator=(const Value& other) -> Value& {
    if (this != &other) {
        clear();
        type_ = other.type_;
        copy_from(other);
    }
    return *this;
}

// Move assignment
auto Value::operator=(Value&& other) noexcept -> Value& {
    if (this != &other) {
        clear();
        type_ = other.type_;

        switch (type_) {
            case ValueType::Int:
                int_val = other.int_val;
                break;
            case ValueType::Float:
                float_val = other.float_val;
                break;
            case ValueType::Bool:
                bool_val = other.bool_val;
                break;
            case ValueType::String:
                string_val = other.string_val;
                other.string_val = nullptr;
                break;
            case ValueType::List:
                list_val = other.list_val;
                other.list_val = nullptr;
                break;
            case ValueType::Tuple:
                tuple_val = other.tuple_val;
                other.tuple_val = nullptr;
                break;
            case ValueType::Function:
                function_val = other.function_val;
                other.function_val.name = nullptr;
                break;
        }
    }
    return *this;
}

// Type conversions
auto Value::as_int() const -> int64_t {
    if (type_ != ValueType::Int) {
        throw std::runtime_error(fmt::format("Expected Int, got {}", type_name()));
    }
    return int_val;
}

auto Value::as_float() const -> double {
    if (type_ != ValueType::Float) {
        throw std::runtime_error(fmt::format("Expected Float, got {}", type_name()));
    }
    return float_val;
}

auto Value::as_bool() const -> bool {
    if (type_ != ValueType::Bool) {
        throw std::runtime_error(fmt::format("Expected Bool, got {}", type_name()));
    }
    return bool_val;
}

auto Value::as_string() const -> const std::string& {
    if (type_ != ValueType::String) {
        throw std::runtime_error(fmt::format("Expected String, got {}", type_name()));
    }
    return *string_val;
}

auto Value::as_list() const -> const std::vector<Value>& {
    if (type_ != ValueType::List) {
        throw std::runtime_error(fmt::format("Expected List, got {}", type_name()));
    }
    return *list_val;
}

auto Value::as_tuple() const -> const std::vector<Value>& {
    if (type_ != ValueType::Tuple) {
        throw std::runtime_error(fmt::format("Expected Tuple, got {}", type_name()));
    }
    return *tuple_val;
}

auto Value::as_function_index() const -> size_t {
    if (type_ != ValueType::Function) {
        throw std::runtime_error(fmt::format("Expected Function, got {}", type_name()));
    }
    return function_val.index;
}

auto Value::as_function_name() const -> const std::string& {
    if (type_ != ValueType::Function) {
        throw std::runtime_error(fmt::format("Expected Function, got {}", type_name()));
    }
    return *function_val.name;
}

// Mutable accessors
auto Value::as_list_mut() -> std::vector<Value>& {
    if (type_ != ValueType::List) {
        throw std::runtime_error(fmt::format("Expected List, got {}", type_name()));
    }
    return *list_val;
}

auto Value::as_tuple_mut() -> std::vector<Value>& {
    if (type_ != ValueType::Tuple) {
        throw std::runtime_error(fmt::format("Expected Tuple, got {}", type_name()));
    }
    return *tuple_val;
}

// Equality
auto Value::operator==(const Value& other) const -> bool {
    if (type_ != other.type_) {
        return false;
    }

    switch (type_) {
        case ValueType::Int:
            return int_val == other.int_val;
        case ValueType::Float:
            return float_val == other.float_val;
        case ValueType::Bool:
            return bool_val == other.bool_val;
        case ValueType::String:
            return *string_val == *other.string_val;
        case ValueType::List:
            return *list_val == *other.list_val;
        case ValueType::Tuple:
            return *tuple_val == *other.tuple_val;
        case ValueType::Function:
            return function_val.index == other.function_val.index;
    }
    return false;
}

// Comparison (for <, >, <=, >=)
auto Value::operator<(const Value& other) const -> bool {
    if (type_ != other.type_) {
        throw std::runtime_error(
            fmt::format("Cannot compare {} and {}", type_name(), other.type_name())
        );
    }

    switch (type_) {
        case ValueType::Int:
            return int_val < other.int_val;
        case ValueType::Float:
            return float_val < other.float_val;
        case ValueType::String:
            return *string_val < *other.string_val;
        default:
            throw std::runtime_error(
                fmt::format("Type {} does not support ordering comparison", type_name())
            );
    }
}

// Truthiness
auto Value::is_truthy() const -> bool {
    switch (type_) {
        case ValueType::Bool:
            return bool_val;
        case ValueType::Int:
            return int_val != 0;
        case ValueType::Float:
            return float_val != 0.0;
        case ValueType::String:
            return !string_val->empty();
        case ValueType::List:
            return !list_val->empty();
        case ValueType::Tuple:
            return !tuple_val->empty();
        case ValueType::Function:
            return true;
    }
    return false;
}

// String representation
auto Value::to_string() const -> std::string {
    switch (type_) {
        case ValueType::Int:
            return fmt::format("{}", int_val);
        case ValueType::Float:
            return fmt::format("{}", float_val);
        case ValueType::Bool:
            return bool_val ? "true" : "false";
        case ValueType::String:
            return fmt::format("\"{}\"", *string_val);
        case ValueType::List: {
            std::string result = "[";
            for (size_t i = 0; i < list_val->size(); ++i) {
                if (i > 0) result += ", ";
                result += (*list_val)[i].to_string();
            }
            result += "]";
            return result;
        }
        case ValueType::Tuple: {
            std::string result = "(";
            for (size_t i = 0; i < tuple_val->size(); ++i) {
                if (i > 0) result += ", ";
                result += (*tuple_val)[i].to_string();
            }
            result += ")";
            return result;
        }
        case ValueType::Function:
            return fmt::format("<function {}>", *function_val.name);
    }
    return "<unknown>";
}

auto Value::type_name() const -> std::string_view {
    switch (type_) {
        case ValueType::Int: return "Int";
        case ValueType::Float: return "Float";
        case ValueType::Bool: return "Bool";
        case ValueType::String: return "String";
        case ValueType::List: return "List";
        case ValueType::Tuple: return "Tuple";
        case ValueType::Function: return "Function";
    }
    return "Unknown";
}

// Helper methods
auto Value::clear() -> void {
    switch (type_) {
        case ValueType::String:
            delete string_val;
            string_val = nullptr;
            break;
        case ValueType::List:
            delete list_val;
            list_val = nullptr;
            break;
        case ValueType::Tuple:
            delete tuple_val;
            tuple_val = nullptr;
            break;
        case ValueType::Function:
            delete function_val.name;
            function_val.name = nullptr;
            break;
        default:
            break;
    }
}

auto Value::copy_from(const Value& other) -> void {
    switch (type_) {
        case ValueType::Int:
            int_val = other.int_val;
            break;
        case ValueType::Float:
            float_val = other.float_val;
            break;
        case ValueType::Bool:
            bool_val = other.bool_val;
            break;
        case ValueType::String:
            string_val = new std::string(*other.string_val);
            break;
        case ValueType::List:
            list_val = new std::vector<Value>(*other.list_val);
            break;
        case ValueType::Tuple:
            tuple_val = new std::vector<Value>(*other.tuple_val);
            break;
        case ValueType::Function:
            function_val.index = other.function_val.index;
            function_val.name = new std::string(*other.function_val.name);
            break;
    }
}

} // namespace lucid::backend
