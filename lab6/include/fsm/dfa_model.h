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

    size_t count_transitions() const {
        size_t count = 0;
        for (const auto& state_transitions : transitions) {
            count += state_transitions.second.size();
        }
        return count;
    }
    bool check_transition(const std::string& from_state, const T& input_char, const std::string& to_state) const {
        try {
        // check if the states/characters are valid
        bool from_state_valid = states_set.find(from_state) != states_set.end();
        bool to_state_valid = states_set.find(to_state) != states_set.end();
        bool input_char_valid = character_set.find(input_char) != character_set.end();
        if (!from_state_valid || !to_state_valid || !input_char_valid) {
            std::string error_msg = "Requesting a transition with invalid states or characters: " + from_state + " --" + input_char + "--> " + to_state;
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        auto it = transitions.find(from_state);
        if (it != transitions.end()) {
            auto it2 = it->second.find(input_char);
            if (it2 != it->second.end() && it2->second == to_state) {
                return true;
            }
        }
        return false;
    }
        catch (const std::exception& e)
        {
            std::string error_msg = "Error checking transition in DFA: " + std::string(e.what());
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
    }
    bool add_transition(const std::string& from_state, const T& input_char, const std::string& to_state) {
        try {
        // check if the transition already exists
        if (check_transition(from_state, input_char, to_state)) {
            spdlog::debug("Transition already exists in DFA: {} --{}--> {}", from_state, input_char, to_state);
            return false; // transition already exists, do nothing
        }
        // check if there is a conflict: same from_state and input_char but different to_state
        auto it = transitions.find(from_state);
        if (it != transitions.end()) {
            auto it2 = it->second.find(input_char);
            if (it2 != it->second.end() && it2->second != to_state) {
                std::string error_msg = "Conflict in DFA: {} --{}--> {} and {} --{}--> {}";
                spdlog::error(error_msg, from_state, input_char, it2->second, from_state, input_char, to_state);
                throw std::runtime_error(error_msg);
            }
        }
        // add the transition to the DFA
        transitions[from_state][input_char] = to_state;
        spdlog::debug("Adding transition to DFA: {} --{}--> {}", from_state, input_char, to_state);
        return true;
    }
    catch (const std::exception& e)
        {
            std::string error_msg = "Error adding transition to DFA: " + std::string(e.what());
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
    }
};

// A DFA that can tolerate conflicts, a stepping stone for PDA creation and SLR1 conflict resolution
template<typename T>
struct ConflictTolerantDFA {
    std::unordered_set<T> character_set;                           // 字符集
    std::unordered_set<std::string> states_set;                       // 状态集
    std::string initial_state;                                        // 开始状态
    std::unordered_set<std::string> accepting_states;                 // 接受状态集
    std::unordered_map<std::string, std::multimap<T, std::string>> transitions; // 状态转换表

    // count the total number of transitions in the DFA
    size_t count_transitions() const {
        size_t count = 0;
        for (const auto& state_transitions : transitions) {
            count += state_transitions.second.size();
        }
        return count;
    }

    bool check_transition(const std::string& from_state, const T& input_char, const std::string& to_state) const {
        try{
        // check if the states/characters are valid
        bool from_state_valid = states_set.find(from_state) != states_set.end();
        bool to_state_valid = states_set.find(to_state) != states_set.end();
        bool input_char_valid = character_set.find(input_char) != character_set.end();
        if (!from_state_valid || !to_state_valid || !input_char_valid) {
            std::string error_msg = "Requesting a transition with invalid states or characters: " + from_state + " --" + input_char + "--> " + to_state;
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        auto it = transitions.find(from_state);
        if (it != transitions.end()) {
            auto range = it->second.equal_range(input_char);
            for (auto iter = range.first; iter != range.second; ++iter) {
                if (iter->second == to_state) {
                    return true;
                }
            }
        }
        return false;
        }
        catch (const std::exception& e)
        {
            std::string error_msg = "Error checking transition in DFA: " + std::string(e.what());
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
    }

    bool add_transition(const std::string& from_state, const T& input_char, const std::string& to_state) {
        try{
            // check if the transition already exists
            if (check_transition(from_state, input_char, to_state)) {
                spdlog::debug("Transition already exists in DFA: {} --{}--> {}", from_state, input_char, to_state);
                return false; // transition already exists, do nothing
            }
            // add the transition to the DFA
            transitions[from_state].insert({input_char, to_state});
            spdlog::debug("Adding transition to DFA: {} --{}--> {}", from_state, input_char, to_state);
            return true;
        }
        catch (const std::exception& e)
        {
            std::string error_msg = "Error adding transition to DFA: " + std::string(e.what());
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
    }
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
