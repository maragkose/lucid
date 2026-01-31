#pragma once

#include <lucid/semantic/type_system.hpp>
#include <lucid/frontend/token.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace lucid {
namespace semantic {

// ===== Symbol Kind =====

enum class SymbolKind {
    Variable,
    Function,
    Parameter
};

// ===== Symbol =====

class Symbol {
public:
    std::string name;
    SymbolKind kind;
    std::unique_ptr<SemanticType> type;
    SourceLocation location;
    bool is_mutable;  // For variables (let vs var - future feature)

    Symbol(std::string name, SymbolKind kind,
           std::unique_ptr<SemanticType> type,
           SourceLocation location, bool is_mutable = false)
        : name(std::move(name)), kind(kind), type(std::move(type)),
          location(location), is_mutable(is_mutable) {}

    // Delete copy, allow move
    Symbol(const Symbol&) = delete;
    Symbol& operator=(const Symbol&) = delete;
    Symbol(Symbol&&) = default;
    Symbol& operator=(Symbol&&) = default;
};

// ===== Scope =====

class Scope {
public:
    enum class ScopeKind {
        Global,
        Function,
        Block,
        Lambda
    };

    ScopeKind kind;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
    Scope* parent;  // Pointer to parent scope (nullptr for global)

    explicit Scope(ScopeKind kind, Scope* parent = nullptr)
        : kind(kind), parent(parent) {}

    // Delete copy, allow move
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = default;
    Scope& operator=(Scope&&) = default;

    // Declare a symbol in this scope
    auto declare(std::string name, SymbolKind kind,
                 std::unique_ptr<SemanticType> type,
                 SourceLocation location,
                 bool is_mutable = false) -> bool;

    // Lookup symbol in this scope only (no parent search)
    auto lookup_local(const std::string& name) -> Symbol*;
    auto lookup_local(const std::string& name) const -> const Symbol*;

    // Check if symbol exists in this scope
    auto exists_local(const std::string& name) const -> bool;
};

// ===== Symbol Table =====

class SymbolTable {
public:
    SymbolTable();

    // Scope management
    auto enter_scope(Scope::ScopeKind kind) -> void;
    auto exit_scope() -> void;
    auto current_scope() -> Scope*;
    auto current_scope() const -> const Scope*;

    // Symbol operations
    auto declare(std::string name, SymbolKind kind,
                 std::unique_ptr<SemanticType> type,
                 SourceLocation location,
                 bool is_mutable = false) -> bool;

    auto lookup(const std::string& name) -> Symbol*;
    auto lookup(const std::string& name) const -> const Symbol*;

    // Check if symbol exists in current or parent scopes
    auto exists(const std::string& name) const -> bool;

    // Check if symbol exists in current scope only
    auto exists_in_current_scope(const std::string& name) const -> bool;

    // Get scope depth (0 = global, 1 = first nested scope, etc.)
    auto scope_depth() const -> size_t;

private:
    std::vector<std::unique_ptr<Scope>> scopes_;
    Scope* current_;
};

// ===== Symbol Table Utilities =====

// Helper to create a function symbol with parameter and return types
auto make_function_symbol(
    std::string name,
    std::vector<std::unique_ptr<SemanticType>> param_types,
    std::unique_ptr<SemanticType> return_type,
    SourceLocation location
) -> std::unique_ptr<Symbol>;

} // namespace semantic
} // namespace lucid
