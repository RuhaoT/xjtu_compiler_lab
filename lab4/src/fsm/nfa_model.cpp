#include "nfa_model.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

bool nfa_model_helper::check_nfa_configuration(const nfa_model::NFA& nfa)
{
    // Check for empty character set
    if (nfa.character_set.empty()) {
        spdlog::debug("NFA has an empty character set");
        return false;
    }

    // Check for empty states set
    if (nfa.states.empty()) {
        spdlog::debug("NFA has an empty states set");
        return false;
    }

    // Check for empty start state
    if (nfa.start_state.empty()) {
        spdlog::debug("NFA has an empty start state");
        return false;
    }

    // Check for empty accepting states
    if (nfa.accepting_states.empty()) {
        spdlog::debug("NFA has an empty accepting states set");
        return false;
    }

    // Check if the start state is in the states set
    if (nfa.states.find(nfa.start_state) == nfa.states.end()) {
        spdlog::debug("NFA has a start state not in the states set: {}", nfa.start_state);
        return false;
    }

    // Check if the accepting states are in the states set
    for (const auto& state : nfa.accepting_states) {
        if (nfa.states.find(state) == nfa.states.end()) {
            spdlog::debug("NFA has an accepting state not in the states set: {}", state);
            return false;
        }
    }

    // note the epsilon/non-epsilon transitions can be empty
    if (!nfa.non_epsilon_transitions.empty()) {
        // Check if the non-epsilon transitions are valid
        for (const auto& transition : nfa.non_epsilon_transitions) {
            const auto& from_state = transition.first;
            const auto& transitions = transition.second;

            // Check if the from state is in the states set
            if (nfa.states.find(from_state) == nfa.states.end()) {
                spdlog::debug("NFA has a non-epsilon transition from a state not in the states set: {}", from_state);
                return false;
            }

            // iterate through each transition
            for (const auto& char_state_pair : transitions) {
                const std::string& input_char = char_state_pair.first;
                const std::string& to_state = char_state_pair.second;

                // Check if the input character is in the character set
                if (nfa.character_set.find(input_char) == nfa.character_set.end()) {
                    spdlog::debug("NFA has a non-epsilon transition with an input character not in the character set: {}", input_char);
                    return false;
                }

                // Check if the to state is in the states set
                if (nfa.states.find(to_state) == nfa.states.end()) {
                    spdlog::debug("NFA has a non-epsilon transition to a state not in the states set: {}", to_state);
                    return false;
                }
            }
        }
    }
    
    if (!nfa.epsilon_transitions.empty()) {
        // Check if the epsilon transitions are valid
        for (const auto& transition : nfa.epsilon_transitions) {
            const auto& from_state = transition.first;
            const auto& to_states = transition.second;

            // Check if the from state is in the states set
            if (nfa.states.find(from_state) == nfa.states.end()) {
                spdlog::debug("NFA has an epsilon transition from a state not in the states set: {}", from_state);
                return false;
            }

            // Check if the to states are in the states set
            if (nfa.states.find(to_states) == nfa.states.end()) {
                spdlog::debug("NFA has an epsilon transition to a state not in the states set: {}", to_states);
                return false;
            }
        }
    }
    spdlog::debug("NFA configuration is valid");
    return true;
}