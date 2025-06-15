#include "syntax_semantic_analyzer/scope_table.h"

void ScopeTable::reset() {
    scope_id_counter = 0;
    scope_stack.clear();
    child_scope_map.clear();
    parent_scope_map.clear();
    scope_stack.push_back(0); // 最外层作用域编号为0
}

void ScopeTable::enterNewScope() {
    int parent_scope = getCurrentScope();
    int new_scope_id = ++scope_id_counter;
    scope_stack.push_back(new_scope_id);
    // 记录父子关系
    child_scope_map[parent_scope].push_back(new_scope_id);
    parent_scope_map[new_scope_id] = parent_scope;
}

void ScopeTable::exitCurrentScope() {
    if (scope_stack.size() <= 1) {
        throw std::runtime_error("Cannot exit global scope");
    }
    scope_stack.pop_back();
}

std::vector<int> ScopeTable::getDirectChildScopes(int scope_id) const {
    auto it = child_scope_map.find(scope_id);
    if (it != child_scope_map.end()) {
        return it->second;
    }
    return {};
}

int ScopeTable::getDirectParentScope(int scope_id) const {
    auto it = parent_scope_map.find(scope_id);
    if (it != parent_scope_map.end()) {
        return it->second;
    }
    return -1;
}

int ScopeTable::getCurrentScope() const {
    if (scope_stack.empty()) {
        throw std::runtime_error("Scope stack is empty");
    }
    return scope_stack.back();
}