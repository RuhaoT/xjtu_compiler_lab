#include "standard_nfa_ctdfa_converter.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>

// constructor & destructor, all default
StandardNFA_DFA_Converter::StandardNFA_DFA_Converter() = default;
StandardNFA_DFA_Converter::~StandardNFA_DFA_Converter() = default;

bool std_nfa_ctdfa_converter_helper::is_subset(const NFAClosure &closure1, const NFAClosure &closure2)
{
    // check if closure1 is a subset of closure2
    for (const auto &state : closure1.states)
    {
        if (closure2.states.find(state) == closure2.states.end())
        {
            return false;
        }
    }
    return true;
}

NFACTDFAConvertionResult StandardNFA_DFA_Converter::convert_nfa_to_dfa(const nfa_model::NFA &nfa)
{
    try
    {
        // check if the NFA is valid
        if (!nfa_model_helper::check_nfa_configuration(nfa))
        {
            std::string error_message = "Error: NFA is not valid";
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
        // auxiliary variables
        NFACTDFAConvertionResult result;

        // step 1: generate the closure set
        std::unordered_set<std_nfa_ctdfa_converter_helper::NFAClosure> closure_set = std_nfa_ctdfa_converter_helper::generate_closure_set(nfa);

        // step 2: generate the state mapping
        NFADFABidirectionalMapping state_mapping = std_nfa_ctdfa_converter_helper::generate_state_mapping(closure_set);

        // step 3: generate the DFA transitions
        std::unordered_map<std::string, std::multimap<std::string, std::string>> dfa_transitions = std_nfa_ctdfa_converter_helper::generate_dfa_transitions(nfa, state_mapping);

        // step 4: build the DFA
        dfa_model::ConflictTolerantDFA<std::string> dfa;
        dfa.character_set = nfa.character_set;
        // iterate through the closure set and build the DFA states
        for (const auto &closure : closure_set)
        {
            // add the closure name to the DFA states
            dfa.states_set.insert(closure.closure_name);
            // check if the closure has an accepting state, if so, add it to the DFA accepting states
            if (closure.has_accepting_state)
            {
                dfa.accepting_states.insert(closure.closure_name);
                spdlog::debug("DFA accepting state: {}", closure.closure_name);
            }
            // check if the closure has the initial state, if so, set it as the DFA initial state
            if (closure.has_initial_state)
            {
                dfa.initial_state = closure.closure_name;
                spdlog::debug("DFA initial state: {}", closure.closure_name);
            }
        }
        // add the DFA transitions
        dfa.transitions = dfa_transitions;
        // check the DFA configurations
        dfa_model_helper::check_conflict_tolerant_dfa_configuration(dfa);
        // update the result
        result.dfa = dfa;
        result.state_mapping = state_mapping;
        spdlog::debug("DFA generation completed");
        // return the result
        return result;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error converting NFA to DFA: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

// helper functions
std::string std_nfa_ctdfa_converter_helper::generate_unique_state_name(const std::unordered_set<std::string> &nfa_state_names)
{
    try
    {
        // generate a unique state name
        // design goal: as long as the state name set is unique, the state name is unique
        // use the simplest way: concatenate all state names with a separator
        std::string unique_state_name;
        unique_state_name += "[\n";
        for (const auto &state_name : nfa_state_names)
        {
            unique_state_name += state_name + "\n";
        }
        if (!unique_state_name.empty())
        {
            ;
        }
        else
        {
            throw std::runtime_error("State name set is empty");
        }
        unique_state_name += "]";
        return unique_state_name;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating unique state name: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

std_nfa_ctdfa_converter_helper::NFAClosure std_nfa_ctdfa_converter_helper::get_epsilon_closure(const nfa_model::NFA &nfa, const std::string &state)
{
    try
    {
        // auxiliary variables
        std::unordered_set<std::string> current_closure_states;
        std::unordered_set<std::string> nfa_remaining_states;
        bool has_accepting_state = false;
        bool has_initial_state = false;
        std::string closure_name;
        int closure_state_increment = 1;
        // initialize the closure state
        // put the initial state into the closure
        current_closure_states.insert(state);
        // put all the NFA states into the remaining states and delete the initial state
        nfa_remaining_states = nfa.states;
        nfa_remaining_states.erase(state);
        // start growing the closure
        while (closure_state_increment > 0)
        {
            closure_state_increment = 0;
            // iterate through the current closure states
            for (const auto &current_state : current_closure_states)
            {
                // check if the current state has epsilon transitions
                if (nfa.epsilon_transitions.find(current_state) != nfa.epsilon_transitions.end())
                {
                    // iterate through the epsilon transitions
                    for (const auto &epsilon_transition : nfa.epsilon_transitions)
                    {
                        if (epsilon_transition.first != current_state)
                        {
                            continue;
                        }
                        // get the epsilon transition state
                        std::string epsilon_transition_state = epsilon_transition.second;
                        // check if the connected state is not in the closure
                        if (current_closure_states.find(epsilon_transition_state) == current_closure_states.end())
                        {
                            spdlog::debug("Adding state {} to closure from state {}", epsilon_transition_state, current_state);
                            // add the state to the closure
                            current_closure_states.insert(epsilon_transition_state);
                            // remove the state from the remaining states
                            nfa_remaining_states.erase(epsilon_transition_state);
                            // increment the closure state increment
                            closure_state_increment++;
                        }
                    }
                }
            }
            spdlog::debug("The closure has grown by {} states to {} states", closure_state_increment, current_closure_states.size());
        }
        // check if the closure has any accepting states
        for (const auto &accepting_state : nfa.accepting_states)
        {
            if (current_closure_states.find(accepting_state) != current_closure_states.end())
            {
                has_accepting_state = true;
                spdlog::debug("The closure has an accepting state: {}", accepting_state);
                break;
            }
        }
        // check if the closure has the initial state
        if (current_closure_states.find(nfa.start_state) != current_closure_states.end())
        {
            has_initial_state = true;
            spdlog::debug("The closure has the initial state: {}", nfa.start_state);
        }
        // generate the closure name
        closure_name = generate_unique_state_name(current_closure_states);
        spdlog::debug("The closure name is {}", closure_name);
        // construct the closure & return it
        NFAClosure closure;
        closure.states = current_closure_states;
        closure.closure_name = closure_name;
        closure.has_accepting_state = has_accepting_state;
        closure.has_initial_state = has_initial_state;
        spdlog::debug("Closure finished: {} with {} states", closure_name, current_closure_states.size());
        return closure;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error getting epsilon closure for state " + state + ": ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

std::unordered_set<std_nfa_ctdfa_converter_helper::NFAClosure>
std_nfa_ctdfa_converter_helper::generate_closure_set(const nfa_model::NFA &nfa)
{
    try
    {
        spdlog::debug("Generating closure set for NFA.");
        std::unordered_set<NFAClosure> closure_set;
        // 1. generate the closure for each NFA state
        for (const auto &state : nfa.states)
        {
            NFAClosure closure = get_epsilon_closure(nfa, state);
            // check for closure with the same name
            if (closure_set.find(closure) != closure_set.end())
            {
                std::string error_message = "The closure set already contains a closure with the same name: " + closure.closure_name;
                spdlog::error(error_message);
                throw std::runtime_error(error_message);
            }
            // add the closure to the closure set
            closure_set.insert(closure);
            spdlog::debug("Added closure {} with {} states to the closure set", closure.closure_name, closure.states.size());
        }

        // 2. remove subset closures
        // iterate through the closure set
        std::unordered_set<NFAClosure> closure_to_remove;
        for (const auto &closure1 : closure_set)
        {
            for (const auto &closure2 : closure_set)
            {
                // if the closures are the same, skip
                if (closure1 == closure2)
                {
                    continue;
                }
                // check if closure1 is a subset of closure2
                if (is_subset(closure1, closure2))
                {
                    // if so, add closure1 to the closure to remove set
                    closure_to_remove.insert(closure1);
                    spdlog::debug("Closure {} is marked for removal from the closure set", closure1.closure_name);
                }
                // check if closure2 is a subset of closure1
                if (is_subset(closure2, closure1))
                {
                    // if so, add closure2 to the closure to remove set
                    closure_to_remove.insert(closure2);
                    spdlog::debug("Closure {} is marked for removal from the closure set", closure2.closure_name);
                }
            }
        }

        // remove the closures from the closure set
        for (const auto &closure : closure_to_remove)
        {
            closure_set.erase(closure);
        }
        spdlog::debug("Removed {} closures from the closure set", closure_to_remove.size());
        
        // check if the closure set is empty
        if (closure_set.empty())
        {
            std::string error_message = "The closure set is empty";
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
        else
        {
            spdlog::debug("Closure set generated with {} closures:", closure_set.size());
            for (const auto &closure : closure_set)
            {
                spdlog::debug("\tClosure: {}", closure.closure_name);
            }
        }
        // return the closure set
        return closure_set;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating closure set: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

NFADFABidirectionalMapping std_nfa_ctdfa_converter_helper::generate_state_mapping(const std::unordered_set<NFAClosure> &closure_set)
{
    try
    {
        spdlog::debug("Generating state mapping:");
        NFADFABidirectionalMapping state_mapping;
        // iterate through the closure set
        for (const auto &closure : closure_set)
        {
            // generate the DFA state name
            std::string dfa_state_name = closure.closure_name;
            // add the DFA state to the mapping
            state_mapping.dfa_to_nfa_mapping[dfa_state_name] = closure.states;
            // iterate through the NFA states
            for (const auto &nfa_state : closure.states)
            {
                // add the NFA state to the mapping
                state_mapping.nfa_to_dfa_mapping[nfa_state] = dfa_state_name;
            }
            spdlog::debug("\tDFA state: {} -> NFA states: {}", dfa_state_name, fmt::join(closure.states, ", "));
        }
        return state_mapping;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating state mapping: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

std::unordered_map<std::string, std::multimap<std::string, std::string>> std_nfa_ctdfa_converter_helper::generate_dfa_transitions(const nfa_model::NFA &nfa, const NFADFABidirectionalMapping &state_mapping)
{
    try
    {
        spdlog::debug("Generating DFA transitions:");
        std::unordered_map<std::string, std::multimap<std::string, std::string>> dfa_transitions;
        // iterate through the NFA states
        for (const auto &nfa_state : nfa.states)
        {
            // find the corresponding DFA state
            if (state_mapping.nfa_to_dfa_mapping.find(nfa_state) == state_mapping.nfa_to_dfa_mapping.end())
            {
                std::string error_message = "The corresponding DFA state of NFA state " + nfa_state + " not found in state mapping";
                spdlog::error(error_message);
                throw std::runtime_error(error_message);
            }
            std::string dfa_state = state_mapping.nfa_to_dfa_mapping.at(nfa_state);
            // iterate through the NFA transitions
            // only need to check the non-epsilon transitions
            // first check if the NFA state has any non-epsilon transitions
            if (nfa.non_epsilon_transitions.find(nfa_state) == nfa.non_epsilon_transitions.end())
            {
                spdlog::debug("No non-epsilon transitions for NFA state {}", nfa_state);
                continue;
            }
            for (const auto &transition : nfa.non_epsilon_transitions.at(nfa_state))
            {
                std::string input_string = transition.first;
                // find the corresponding DFA state
                if (state_mapping.nfa_to_dfa_mapping.find(transition.second) == state_mapping.nfa_to_dfa_mapping.end())
                {
                    std::string error_message = "The corresponding DFA state of NFA state " + transition.second + " not found in state mapping";
                    spdlog::error(error_message);
                    throw std::runtime_error(error_message);
                }
                std::string dfa_next_state = state_mapping.nfa_to_dfa_mapping.at(transition.second);
                // add the transition to the DFA transitions
                dfa_transitions[dfa_state].insert({input_string, dfa_next_state});
                spdlog::debug("\tDFA transition: {} --{}--> {}", dfa_state, input_string, dfa_next_state);
            }
        }
        return dfa_transitions;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating DFA transitions: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}