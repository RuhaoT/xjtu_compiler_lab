#ifndef SYNTAX_SEMANTIC_MODEL_H
#define SYNTAX_SEMANTIC_MODEL_H

#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "spdlog/spdlog.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace syntax_semantic_model {

// 本质上是个复杂的多级map，表示每个非终结符的产生式列表
struct ProductionInfoMapping {
    std::unordered_map<cfg_model::symbol, std::unordered_map<std::vector<cfg_model::symbol>, std::string>> production_info; // 产生式信息，键为非终结符，值为一个map，键为产生式右侧符号序列，值为节点类型

    // 提供一个包装好的查询方法，能够应对没找到的情况（报错）
    std::string get_node_type(const cfg_model::symbol& lhs, const std::vector<cfg_model::symbol>& rhs) const;
};

enum class SymbolType {
    Variable,
    Array,
    Function
};

struct SymbolEntry {
    std::string symbol_name; // 符号名
    SymbolType symbol_type;  // 符号类型
    int scope_id;            // 作用域id
    std::string data_type;   // 数据类型
    int memory_size;         // 抽象内存大小

    // 可选特殊属性
    std::optional<int> array_length; // 数组长度，仅对数组符号有效
    std::optional<std::vector<std::string>> arg_list; // 函数形参列表，仅对函数符号有效
    std::optional<int> direct_child_scope; // 直接子作用域，仅对函数符号有效

    // 支持set排序
    bool operator<(const SymbolEntry& other) const {
        if (scope_id != other.scope_id) return scope_id < other.scope_id;
        return symbol_name < other.symbol_name;
    }
};



}

#endif // !SYNTAX_SEMANTIC_MODEL_H