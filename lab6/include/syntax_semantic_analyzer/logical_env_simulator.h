#ifndef LOGICAL_ENV_SIMULATOR_H
#define LOGICAL_ENV_SIMULATOR_H

#include "interm_code/interm_code_model.h"
#include "symbol_table.h"
#include "syntax_semantic_model.h"
#include <unordered_map>
#include <map>
#include <vector>

class LogicalEnvSimulator
{
public:                                                                                                      // attributes, set public for easy access in tests
    std::unordered_set<int> scope_ids;                                                                       // 作用域ID集合
    std::unordered_map<int, int> scope_treg_usage;                                                           // 每个作用域的T寄存器使用情况
    std::unordered_map<int, std::unordered_map<std::string, interm_code_model::Register>> scope_var_reg_map; // 每个作用域的变量寄存器映射
    std::unordered_map<int, std::unordered_map<std::string, interm_code_model::Register>> scope_arr_reg_map; // 每个作用域的数组的基地址存储情况
    std::multimap<int, std::string> scope_label_map;                                                         // 每个作用域的标签使用情况
    std::unordered_set<std::string> total_labels;                                                            // 全局标签集合
    std::unordered_map<std::string, std::string> label_to_func_map;                                          // 标签到函数的映射
    SymbolTable symbol_table;                                                                                // 符号表
    int stack_usage = 0;                                                                                     // 栈空间使用情况，单位是逻辑寄存器
    int memory_usage = 0;                                                                                    // 内存使用情况，单位是最小内存单元
    std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> interm_code_list;                      // 中间代码列表

public:
    LogicalEnvSimulator() = default;
    LogicalEnvSimulator(const SymbolTable &symbol_table);

    void init();

    // check if the given scope ID exists
    void check_scope_id(int scope_id);

    // check if the given variable, array, or function exists in the current scope
    void check_symbol_table_var(const std::string &var_name, int scope_id);
    void check_symbol_table_arr(const std::string &arr_name, int scope_id);
    void check_symbol_table_func(const std::string &func_name);

    // check if the given variable is mapped to a register in the current scope
    void check_reg_mapping_var(const std::string &var_name, int scope_id);
    void check_reg_mapping_arr(const std::string &arr_name, int scope_id);
    void check_func_mapped_to_label(const std::string &func_name);

    // get a new T register for the given scope
    interm_code_model::Register get_new_treg(int scope_id);
    // get a new temp label for the given scope
    std::string get_new_temp_label(int scope_id);
    // get the reg for the given variable in the given scope
    interm_code_model::Register get_var_reg(const std::string &var_name, int scope_id);
    // get the reg for the given array's initial memory address in the given scope
    interm_code_model::Register get_arr_reg(const std::string &arr_name, int scope_id);
    // get the label for the given function
    std::string get_func_label(const std::string &func_name);

    // register a variable in the scope
    void register_var(const std::string &var_name, int scope_id, interm_code_model::Register reg);
    // register an array in the scope
    void register_arr(const std::string &arr_name, int scope_id, interm_code_model::Register reg);
    // register a function label
    void register_func_label(const std::string &func_name, const std::string &label);

    // save the current state of a given scope
    std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> save_scope_state(int scope_id);
    // restore the state of a given scope
    std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> restore_scope_state(int scope_id);
    // add a new intermediate code batch to the simulator
    void add_intermediate_code_batch(const std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> &interm_code_batch);
    // get the current intermediate code list
    std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> get_intermediate_code_list() const;
};

#endif // !LOGICAL_ENV_SIMULATOR_H