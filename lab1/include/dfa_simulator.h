#ifndef DFA_SIMULATOR_H
#define DFA_SIMULATOR_H

#include <string>
#include <set>
#include "dfa_model.h"

class DFASimulator {
public:
    virtual ~DFASimulator() = default;

    // 更新DFA设定
    virtual bool UpdateDFA(const DFA& dfa) = 0;

    // 模拟单一字符串
    virtual bool SimulateString(const std::string& input, std::string& simulation_log) const = 0;

    // 生成所有符合规则的字符串
    virtual std::set<std::string> GenerateAcceptedStrings(int max_length) const = 0;
};


#endif // !DFA_SIMULATOR_H