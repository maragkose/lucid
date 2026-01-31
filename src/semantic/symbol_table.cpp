#include <lucid/semantic/symbol_table.hpp>

namespace lucid {
namespace semantic {

// ===== Scope Implementation =====

auto Scope::declare(std::string name, SymbolKind kind,
                    std::unique_ptr<SemanticType> type,
                    SourceLocation location,
                    bool is_mutable) -> bool {
    // Check if symbol already exists in this scope
    if (symbols.find(name) != symbols.end()) {
        return false;  // Redeclaration error
    }

    // Create and insert the symbol
    auto symbol = std::make_unique<Symbol>(
        name, kind, std::move(type), location, is_mutable
    );
    symbols[name] = std::move(symbol);
    return true;
}

auto Scope::lookup_local(const std::string& name) -> Symbol* {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    return nullptr;
}

auto Scope::lookup_local(const std::string& name) const -> const Symbol* {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    return nullptr;
}

auto Scope::exists_local(const std::string& name) const -> bool {
    return symbols.find(name) != symbols.end();
}

// ===== SymbolTable Implementation =====

SymbolTable::SymbolTable() {
    // Create global scope
    auto global_scope = std::make_unique<Scope>(Scope::ScopeKind::Global);
    current_ = global_scope.get();
    scopes_.push_back(std::move(global_scope));
}

auto SymbolTable::enter_scope(Scope::ScopeKind kind) -> void {
    auto new_scope = std::make_unique<Scope>(kind, current_);
    current_ = new_scope.get();
    scopes_.push_back(std::move(new_scope));
}

auto SymbolTable::exit_scope() -> void {
    if (current_->parent != nullptr) {
        current_ = current_->parent;
        // Note: We keep all scopes in scopes_ for debugging/analysis
        // They'll be cleaned up when SymbolTable is destroyed
    }
}

auto SymbolTable::current_scope() -> Scope* {
    return current_;
}

auto SymbolTable::current_scope() const -> const Scope* {
    return current_;
}

auto SymbolTable::declare(std::string name, SymbolKind kind,
                          std::unique_ptr<SemanticType> type,
                          SourceLocation location,
                          bool is_mutable) -> bool {
    return current_->declare(std::move(name), kind, std::move(type),
                            location, is_mutable);
}

auto SymbolTable::lookup(const std::string& name) -> Symbol* {
    // Search from current scope up to global scope
    Scope* scope = current_;
    while (scope != nullptr) {
        auto* symbol = scope->lookup_local(name);
        if (symbol != nullptr) {
            return symbol;
        }
        scope = scope->parent;
    }
    return nullptr;
}

auto SymbolTable::lookup(const std::string& name) const -> const Symbol* {
    // Search from current scope up to global scope
    const Scope* scope = current_;
    while (scope != nullptr) {
        auto* symbol = scope->lookup_local(name);
        if (symbol != nullptr) {
            return symbol;
        }
        scope = scope->parent;
    }
    return nullptr;
}

auto SymbolTable::exists(const std::string& name) const -> bool {
    return lookup(name) != nullptr;
}

auto SymbolTable::exists_in_current_scope(const std::string& name) const -> bool {
    return current_->exists_local(name);
}

auto SymbolTable::scope_depth() const -> size_t {
    size_t depth = 0;
    const Scope* scope = current_;
    while (scope->parent != nullptr) {
        depth++;
        scope = scope->parent;
    }
    return depth;
}

// ===== Symbol Table Utilities =====

auto make_function_symbol(
    std::string name,
    std::vector<std::unique_ptr<SemanticType>> param_types,
    std::unique_ptr<SemanticType> return_type,
    SourceLocation location
) -> std::unique_ptr<Symbol> {

    auto func_type = std::make_unique<FunctionType>(
        std::move(param_types),
        std::move(return_type)
    );

    return std::make_unique<Symbol>(
        std::move(name),
        SymbolKind::Function,
        std::move(func_type),
        location,
        false  // Functions are not mutable
    );
}

} // namespace semantic
} // namespace lucid
