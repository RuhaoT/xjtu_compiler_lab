#ifndef DFA_MODEL_H
#define DFA_MODEL_H

#include "spdlog/spdlog.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

// DFA model
// a more versatile DFA model with template
namespace dfa_model {

template<typename T>
struct DFA {
    std::unordered_set<T> character_set;                           // 字符集
    std::unordered_set<std::string> states_set;                       // 状态集
    std::string initial_state;                                        // 开始状态
    std::unordered_set<std::string> accepting_states;                 // 接受状态集
    std::unordered_map<std::string, std::unordered_map<T, std::string>> transitions; // 状态转换表
};

// A DFA that can tolerate conflicts, a stepping stone for PDA creation and SLR1 conflict resolution
template<typename T>
struct ConflictTolerantDFA {
    std::unordered_set<T> character_set;                           // 字符集
    std::unordered_set<std::string> states_set;                       // 状态集
    std::string initial_state;                                        // 开始状态
    std::unordered_set<std::string> accepting_states;                 // 接受状态集
    std::unordered_map<std::string, std::multimap<T, std::string>> transitions; // 状态转换表
};
}

// helper function for DFA
namespace dfa_model_helper
{
    // generate a unique state name for a given state
    template<typename T>
    void check_dfa_configuration(const dfa_model::DFA<T>& dfa)
    {
        std::string dfa_name = dfa.initial_state;
        spdlog::debug("Checking DFA: {}", dfa_name);
        // Check for empty character set
        if (dfa.character_set.empty()) {
            spdlog::debug("DFA has an empty character set", dfa_name);
            throw std::runtime_error("DFA has an empty character set");
        }

        // Check for empty states set
        if (dfa.states_set.empty()) {
            spdlog::debug("DFA has an empty states set", dfa_name);
            throw std::runtime_error("DFA has an empty states set");
        }

        // Check for empty initial state
        if (dfa.initial_state.empty()) {
            spdlog::debug("DFA has an empty initial state", dfa_name);
            throw std::runtime_error("DFA has an empty initial state");
        }

        // Check for empty accepting states
        if (dfa.accepting_states.empty()) {
            spdlog::debug("DFA has an empty accepting states set", dfa_name);
            throw std::runtime_error("DFA has an empty accepting states set");
        }

        // check if the initial state is in the states set
        if (dfa.states_set.find(dfa.initial_state) == dfa.states_set.end()) {
            spdlog::debug("DFA has an initial state not in the states set : {}", dfa.initial_state);
            throw std::runtime_error("DFA has an initial state not in the states set" + dfa.initial_state);
        }

        // check if the accepting states are in the states set
        for (const auto& accepting_state : dfa.accepting_states) {
            if (dfa.states_set.find(accepting_state) == dfa.states_set.end()) {
                spdlog::debug("DFA has an accepting state not in the states set: {}", accepting_state);
                throw std::runtime_error("DFA has an accepting state not in the states set: " + accepting_state);
            }
        }

        // iterate through each transition
        for (const auto& transition_pair : dfa.transitions) {
            const std::string& from_state = transition_pair.first;
            const auto& transitions = transition_pair.second;

            // Check if the from state is in the states set
            if (dfa.states_set.find(from_state) == dfa.states_set.end()) {
                spdlog::debug("DFA has a transition from a state not in the states set: {}", from_state);
                throw std::runtime_error("DFA has a transition from a state not in the states set: " + from_state);
            }

            // iterate through each character-state pair in the transitions
            for (const auto& char_state_pair : transitions) {
                T input_char = char_state_pair.first;
                const std::string& to_state = char_state_pair.second;

                // Check if the input character is in the character set
                if (dfa.character_set.find(input_char) == dfa.character_set.end()) {
                    spdlog::debug("DFA has a transition on a character not in the character set: {}--{}-->", dfa_name, input_char);
                    throw std::runtime_error("DFA has a transition on a character not in the character set: " + dfa_name + " --" + input_char + "-->");
                }

                // Check if the to state is in the states set
                if (dfa.states_set.find(to_state) == dfa.states_set.end()) {
                    spdlog::debug("DFA has a transition to a state not in the states set: {} --{}--> {}", dfa_name, input_char, to_state);
                    throw std::runtime_error("DFA has a transition to a state not in the states set: " + dfa_name + " --" + input_char + "--> " + to_state);
                }
            }
        }
        spdlog::debug("DFA {} is valid", dfa_name);
    }

    template<typename T>
    void check_conflict_tolerant_dfa_configuration(const dfa_model::ConflictTolerantDFA<T>& dfa)
    {
        std::string dfa_name = dfa.initial_state;
        spdlog::debug("Checking Conflict Tolerant DFA: {}", dfa_name);
        // Check for empty character set
        if (dfa.character_set.empty()) {
            spdlog::debug("Conflict Tolerant DFA has an empty character set", dfa_name);
            throw std::runtime_error("Conflict Tolerant DFA has an empty character set");
        }

        // Check for empty states set
        if (dfa.states_set.empty()) {
            spdlog::debug("Conflict Tolerant DFA has an empty states set", dfa_name);
            throw std::runtime_error("Conflict Tolerant DFA has an empty states set");
        }

        // Check for empty initial state
        if (dfa.initial_state.empty()) {
            spdlog::debug("Conflict Tolerant DFA has an empty initial state", dfa_name);
            throw std::runtime_error("Conflict Tolerant DFA has an empty initial state");
        }

        // Check for empty accepting states
        if (dfa.accepting_states.empty()) {
            spdlog::debug("Conflict Tolerant DFA has an empty accepting states set", dfa_name);
            throw std::runtime_error("Conflict Tolerant DFA has an empty accepting states set");
        }

        // check if the initial state is in the states set
        if (dfa.states_set.find(dfa.initial_state) == dfa.states_set.end()) {
            spdlog::debug("Conflict Tolerant DFA has an initial state not in the states set : {}", dfa.initial_state);
            throw std::runtime_error("Conflict Tolerant DFA has an initial state not in the states set" + dfa.initial_state);
        }

        // check if the accepting states are in the states set
        for (const auto& accepting_state : dfa.accepting_states) {
            if (dfa.states_set.find(accepting_state) == dfa.states_set.end()) {
                spdlog::debug("Conflict Tolerant DFA has an accepting state not in the states set: {}", accepting_state);
                throw std::runtime_error("Conflict Tolerant DFA has an accepting state not in the states set: " + accepting_state);
            }
        }

        // iterate through each transition
        for (const auto& transition_pair : dfa.transitions) {
            const std::string& from_state = transition_pair.first;
            const auto& transitions = transition_pair.second;

            // Check if the from state is in the states set
            if (dfa.states_set.find(from_state) == dfa.states_set.end()) {
                spdlog::debug("Conflict Tolerant DFA has a transition from a state not in the states set: {}", from_state);
                throw std::runtime_error("Conflict Tolerant DFA has a transition from a state not in the states set: " + from_state);
            }

            // iterate through each character-state pair in the transitions
            for (const auto& char_state_pair : transitions) {
                T input_char = char_state_pair.first;
                const std::string& to_state = char_state_pair.second;

                // Check if the input character is in the character set
                if (dfa.character_set.find(input_char) == dfa.character_set.end()) {
                    spdlog::debug("Conflict Tolerant DFA has a transition on a character not in the character set: {}--{}-->", dfa_name, input_char);
                    throw std::runtime_error("Conflict Tolerant DFA has a transition on a character not in the character set: " + dfa_name + " --" + input_char + "-->");
                }

                // Check if the to state is in the states set
                if (dfa.states_set.find(to_state) == dfa.states_set.end()) {
                    spdlog::debug("Conflict Tolerant DFA has a transition to a state not in the states set: {} --{}--> {}", dfa_name, input_char, to_state);
                    throw std::runtime_error("Conflict Tolerant DFA has a transition to a state not in the states set: " + dfa_name + " --" + input_char + "--> " + to_state);
                }
            }
        }
        spdlog::debug("Conflict Tolerant DFA is valid");
    }
}

#endif // DFA_MODEL_H
