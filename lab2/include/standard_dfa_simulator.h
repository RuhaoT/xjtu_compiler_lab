#ifndef STANDARD_DFA_SIMULATOR_H
#define STANDARD_DFA_SIMULATOR_H

#include "dfa_simulator.h"

class StandardDFASimulator : public DFASimulator
{
private:
    DFA dfa;

public:
    StandardDFASimulator() = default;
    ~StandardDFASimulator() override = default;

    // 更新DFA设定
    bool UpdateDFA(const DFA &dfa) override;

    // 模拟单一字符串
    bool SimulateString(const std::string &input, std::string &simulation_log) const override;

    // 生成所有符合规则的字符串
    std::set<std::string> GenerateAcceptedStrings(int max_length) const override;

private:
    // 检查单个字符在单个状态是否能转移
    bool CheckSingleCharInSingleState(const std::string &state, const char input_char) const;

    // 单个字符单步模拟，假设可以转移
    std::string SingleStepSimulate(const std::string &state, const char input_char) const;

    // 检查单个状态是否为接受状态
    bool IsAcceptState(const std::string &state) const;

    // 递归生成符合规则的字符串
    void GenerateAcceptedStrings(const std::string &current_string, const std::string &current_state, int max_length, std::set<std::string> &accepted_strings) const;
};

#endif // !STANDARD_DFA_SIMULATOR_H