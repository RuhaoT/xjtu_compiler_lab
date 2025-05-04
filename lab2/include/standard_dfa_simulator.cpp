#include "standard_dfa_simulator.h"
#include "spdlog/spdlog.h"
#include <iostream>

bool StandardDFASimulator::UpdateDFA(const DFA &dfa) {
    try {
        this->dfa = dfa;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error updating DFA: " << e.what() << std::endl;
        return false;
    }
}

bool StandardDFASimulator::SimulateString(const std::string &input, std::string &simulation_log) const {
    if (input.empty()) {
        std::cerr << "Error: Input string is empty" << std::endl;
        return false;
    }
    // spdlog::debug("Simulating string: {}", input);
    // spdlog::debug("DFA initial state: {}", dfa.initial_state);
    // spdlog::debug("DFA accepting states: {}", fmt::join(dfa.accepting_states, ", "));
    // spdlog::debug("DFA character set: {}", fmt::join(dfa.character_set, ", "));
    // spdlog::debug("DFA transitions: ");
    /* for (const auto &transition : dfa.transitions) {
        spdlog::debug("State {}: ", transition.first);
        for (const auto &char_state_pair : transition.second) {
            spdlog::debug("  {} -> {}", char_state_pair.first, char_state_pair.second);
        }
    } */

    std::string current_state = dfa.initial_state;
    simulation_log = "Start single string simulation\n";
    simulation_log += "Target string: " + input + "\n";
    simulation_log = "Initial state: " + current_state + "\n";

    // 遍历输入字符串的每个字符
    for (const char &input_char : input) {
        
        // 当前字符
        simulation_log += "Current character: '" + std::string(1, input_char) + "'\n";
        
        // 检查是否可以转移
        if (!CheckSingleCharInSingleState(current_state, input_char)) {
            // 注意这不是程序错误，只是输入字符串不符合DFA
            // 将情况记录到日志中
            simulation_log += "No transition from state " + current_state + " on input '" + input_char + "'\n";
            return false;
        }

        // 进行单步模拟
        current_state = SingleStepSimulate(current_state, input_char);
        simulation_log += "Transition to state: " + current_state + "\n";
    }

    // 检查最终状态是否为接受状态
    simulation_log += "Final state: " + current_state + "\n";
    if (IsAcceptState(current_state)) {
        simulation_log += "Accepted\n";
        spdlog::info("Accepted string: {}, log: {}", input, simulation_log);
        return true;
    } else {
        simulation_log += "Rejected\n";
        spdlog::info("Rejected string: {}, log: {}", input, simulation_log);
        return false;
    }
}

bool StandardDFASimulator::CheckSingleCharInSingleState(const std::string &state, const char input_char) const {
    // 检查当前状态是否存在
    if (dfa.transitions.find(state) == dfa.transitions.end()) {
        return false;
    }

    // 检查输入字符是否存在于转换表中
    if (dfa.transitions.at(state).find(input_char) == dfa.transitions.at(state).end()) {
        return false;
    }

    return true;
}

bool StandardDFASimulator::IsAcceptState(const std::string &state) const {
    return dfa.accepting_states.find(state) != dfa.accepting_states.end();
}

std::string StandardDFASimulator::SingleStepSimulate(const std::string &state, const char input_char) const {
    return dfa.transitions.at(state).at(input_char);
}

std::set<std::string> StandardDFASimulator::GenerateAcceptedStrings(int max_length) const {
    std::set<std::string> accepted_strings;

    // 递归生成符合规则的字符串
    GenerateAcceptedStrings("", dfa.initial_state, max_length, accepted_strings);
    return accepted_strings;
}

void StandardDFASimulator::GenerateAcceptedStrings(const std::string &current_string, const std::string &current_state, int max_length, std::set<std::string> &accepted_strings) const {
    // 如果当前字符串长度超过最大长度，返回
    if (current_string.length() > max_length) {
        return;
    }

    // 如果当前状态是接受状态，添加到结果集中
    if (IsAcceptState(current_state)) {
        accepted_strings.insert(current_string);
    }

    // 遍历所有可能的输入字符
    for (const char &input_char : dfa.character_set) {
        // 检查是否可以转移
        if (CheckSingleCharInSingleState(current_state, input_char)) {
            // 进行单步模拟
            std::string next_state = SingleStepSimulate(current_state, input_char);
            GenerateAcceptedStrings(current_string + input_char, next_state, max_length, accepted_strings);
        }
        // 否则字符串必然不符合规则
        // 不用继续递归
    }
}