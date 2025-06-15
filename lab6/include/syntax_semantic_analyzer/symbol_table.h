#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "cfg_model.h"
#include "syntax_semantic_analyzer/syntax_semantic_model.h"
#include <set>
#include <optional>

class SymbolTable {
public:
    // 添加符号到符号表
    void addSymbol(const syntax_semantic_model::SymbolEntry& symbol);

    // 判断符号是否存在
    bool symbolExists(const std::string& symbol_name, int scope_id) const;

    // 查找某个作用域内的某个符号，包括形参
    std::optional<syntax_semantic_model::SymbolEntry> findSymbolInScope(const std::string& symbol_name, int scope_id) const;

    // 递归链式查询某个符号在所有作用域中的定义
    std::optional<syntax_semantic_model::SymbolEntry> findSymbolInAllScopes(const std::string& symbol_name) const;

    // 获取当前作用域的符号列表（注意：如果在函数体内，还要包括函数形参）
    std::vector<syntax_semantic_model::SymbolEntry> getAllSymbolsInScope(int scope_id) const;

public:
    // 符号表使用set存储符号，按作用域id和符号名的字典序排序
    std::set<syntax_semantic_model::SymbolEntry> symbols;
};

#endif // !SYMBOL_TABLE_H