#ifndef SCOPE_TABLE_H
#define SCOPE_TABLE_H

#include <vector>
#include <unordered_map>
#include <stdexcept>


class ScopeTable {
public:
    // 重置作用域表
    void reset();

    // 进入新作用域，返回新的作用域编号
    void enterNewScope();

    // 退出当前作用域，返回上一个作用域编号
    void exitCurrentScope();

    // 查询直接子作用域
    std::vector<int> getDirectChildScopes(int scope_id) const;

    // 查询直接父作用域
    int getDirectParentScope(int scope_id) const;

    // 获取当前作用域编号
    int getCurrentScope() const;
private:
    int scope_id_counter; // 作用域id计数器
    std::vector<int> scope_stack; // 作用域栈
    std::unordered_map<int, std::vector<int>> child_scope_map; // 子作用域表
    std::unordered_map<int, int> parent_scope_map; // 父作用域表
};

#endif // !SCOPE_TABLE_H