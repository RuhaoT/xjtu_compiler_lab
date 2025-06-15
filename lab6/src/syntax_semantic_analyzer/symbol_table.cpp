#include "symbol_table.h"
#include <algorithm>
#include <stdexcept>

using syntax_semantic_model::SymbolEntry;

void SymbolTable::addSymbol(const SymbolEntry& symbol) {
    // 检查是否已存在同名同作用域符号
    for (const auto& s : symbols) {
        if (s.symbol_name == symbol.symbol_name && s.scope_id == symbol.scope_id) {
            throw std::runtime_error("Duplicate symbol declaration: " + symbol.symbol_name);
        }
    }
    symbols.insert(symbol);
}

bool SymbolTable::symbolExists(const std::string& symbol_name, int scope_id) const {
    for (const auto& s : symbols) {
        if (s.symbol_name == symbol_name && s.scope_id == scope_id) {
            return true;
        }
    }
    return false;
}

std::optional<SymbolEntry> SymbolTable::findSymbolInScope(const std::string& symbol_name, int scope_id) const {
    for (const auto& s : symbols) {
        if (s.symbol_name == symbol_name && s.scope_id == scope_id) {
            return s;
        }
    }
    return std::nullopt;
}

std::optional<SymbolEntry> SymbolTable::findSymbolInAllScopes(const std::string& symbol_name) const {
    // 查找所有作用域，优先返回作用域id最大的（最近的）
    int max_scope = -1;
    std::optional<SymbolEntry> result;
    for (const auto& s : symbols) {
        if (s.symbol_name == symbol_name && s.scope_id > max_scope) {
            max_scope = s.scope_id;
            result = s;
        }
    }
    return result;
}

std::vector<SymbolEntry> SymbolTable::getAllSymbolsInScope(int scope_id) const {
    std::vector<SymbolEntry> result;
    for (const auto& s : symbols) {
        if (s.scope_id == scope_id) {
            result.push_back(s);
        }
    }
    return result;
}