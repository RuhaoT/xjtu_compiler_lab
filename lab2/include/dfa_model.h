#ifndef DFA_MODEL_H
#define DFA_MODEL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// DFA数据结构定义
struct DFA {
    std::unordered_set<char> character_set;                           // 字符集
    std::unordered_set<std::string> states_set;                       // 状态集
    std::string initial_state;                                        // 开始状态
    std::unordered_set<std::string> accepting_states;                 // 接受状态集
    std::unordered_map<std::string, std::unordered_map<char, std::string>> transitions; // 状态转换表
};

#endif // DFA_MODEL_H
