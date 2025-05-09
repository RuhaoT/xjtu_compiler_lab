#ifndef MULTITYPE_DFA_SIMULATOR_H
#define MULTITYPE_DFA_SIMULATOR_H

#include "dfa_simulator.h"
#include "dfa_model.h"
#include "spdlog/spdlog.h"
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename T>
class MultiTypeDFASimulator : public DFASimulator<T>
{
private:
    dfa_model::DFA<T> dfa;
public:
    MultiTypeDFASimulator() = default;
    ~MultiTypeDFASimulator() override = default;

    // update dfa setting
    bool UpdateDFA(const dfa_model::DFA<T>& dfa) override;

    // simulate an array of characters with type T
    bool SimulateString(const std::vector<T>& input) override;
};

namespace multitype_dfa_simulator_helper
{
    // helper function to check if a single character can be transitioned in a single state
    template<typename T>
    bool CheckSingleCharInSingleState(const dfa_model::DFA<T>& dfa, const std::string& state, const T& input_char);

    // simulate a single step, assuming the transition is valid
    template<typename T>
    std::string SingleStepSimulate(const dfa_model::DFA<T>& dfa, const std::string& state, const T& input_char);

    // check if a single state is an accepting state
    template<typename T>
    bool IsAcceptState(const dfa_model::DFA<T>& dfa, const std::string& state);
}

// template class member functions have to be defined in the header file
template<typename T>
bool MultiTypeDFASimulator<T>::UpdateDFA(const dfa_model::DFA<T>& dfa) {
    try {
        this->dfa = dfa;
        return true;
    } catch (const std::exception &e) {
        std::string error_message = "Error updating DFA: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
        return false;
    }
}

template<typename T>
bool MultiTypeDFASimulator<T>::SimulateString(const std::vector<T>& input) {
    try{
        // check if input is empty
        if (input.empty()) {
            spdlog::error("Error: Input vector is empty");
            return false;
        }
        std::string current_state = dfa.initial_state;
        spdlog::debug("Start multi-type string simulation");
        spdlog::debug("Target string: {}", fmt::join(input, ", "));
        spdlog::debug("Initial state: {}", current_state);

        for (const auto &input_char : input) {
            spdlog::debug("Current character: {}", input_char);

            // Check if the transition is valid
            if (!multitype_dfa_simulator_helper::CheckSingleCharInSingleState(dfa, current_state, input_char)) {
                // note this is not considered an error, just a rejection
                spdlog::debug("No transition from state {} on input '{}'", current_state, input_char);
                spdlog::info("Rejected string: {}, reason: {}", fmt::join(input, ", "), "No transition for input character");
                return false;
            }

            // Perform single step simulation
            current_state = multitype_dfa_simulator_helper::SingleStepSimulate(dfa, current_state, input_char);
            spdlog::debug("Transition to state: {}", current_state);
        }

        // Check if the final state is an accepting state
        spdlog::debug("Final state: {}", current_state);
        if (multitype_dfa_simulator_helper::IsAcceptState(dfa, current_state)) {
            spdlog::info("Accepted string: {}, reason: {}", fmt::join(input, ", "), "Final state is accepting");
            return true;
        } else {
            spdlog::info("Rejected string: {}, reason: {}", fmt::join(input, ", "), "Final state is not accepting");
            return false;
        }
    }
    catch (const std::exception &e) {
        std::string error_message = "Error simulating string: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
        return false;
    }
}

template<typename T>
bool multitype_dfa_simulator_helper::CheckSingleCharInSingleState(const dfa_model::DFA<T>& dfa, const std::string& state, const T& input_char) {
    // Check if the state exists in the DFA
    if (dfa.transitions.find(state) == dfa.transitions.end()) {
        return false;
    }
    // Check if the input character can be transitioned from the state
    return dfa.transitions.at(state).find(input_char) != dfa.transitions.at(state).end();
}

template<typename T>
bool multitype_dfa_simulator_helper::IsAcceptState(const dfa_model::DFA<T>& dfa, const std::string& state) {
    // Check if the state is in the accepting states set
    return dfa.accepting_states.find(state) != dfa.accepting_states.end();
}

template<typename T>
std::string multitype_dfa_simulator_helper::SingleStepSimulate(const dfa_model::DFA<T>& dfa, const std::string& state, const T& input_char) {
    // Check if the state exists in the DFA
    if (dfa.transitions.find(state) == dfa.transitions.end()) {
        throw std::runtime_error("State not found in DFA");
    }
    // Check if the input character can be transitioned from the state
    if (dfa.transitions.at(state).find(input_char) == dfa.transitions.at(state).end()) {
        throw std::runtime_error("No transition for input character");
    }
    // Return the next state
    return dfa.transitions.at(state).at(input_char);
}

#endif // !MULTITYPE_DFA_SIMULATOR_H