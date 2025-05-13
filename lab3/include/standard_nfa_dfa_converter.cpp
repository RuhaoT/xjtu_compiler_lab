#include "standard_nfa_dfa_converter.h"
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

NFADFAConvertionResult StandardNFA_DFA_Converter::convert_nfa_to_dfa(const nfa_model::NFA &nfa)
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
        NFADFAConvertionResult result;

        // step 1: generate the closure set
        std::unordered_set<std_nfa_dfa_converter_helper::NFAClosure> closure_set = std_nfa_dfa_converter_helper::generate_closure_set(nfa);

        // step 2: generate the state mapping
        NFADFABidirectionalMapping state_mapping = std_nfa_dfa_converter_helper::generate_state_mapping(closure_set);

        // step 3: build the DFA
        dfa_model::DFA<std::string> dfa;
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
        // step 4: generate the DFA transitions
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> dfa_transitions = std_nfa_dfa_converter_helper::generate_dfa_transitions(nfa, state_mapping, dfa);
        dfa.transitions = dfa_transitions;

        // step 5: check & finish the DFA
        // check the DFA configurations
        dfa_model_helper::check_dfa_configuration(dfa);
        // update the result
        result.dfa = dfa;
        result.state_mapping = state_mapping;
        spdlog::debug("Generated DFA with {} states, {} accepting states, and {} transitions", dfa.states_set.size(), dfa.accepting_states.size(), dfa.count_transitions());
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
std::string std_nfa_dfa_converter_helper::generate_unique_state_name(const std::unordered_set<std::string> &nfa_state_names)
{
    try
    {
        // generate a unique state name
        // design goal: as long as the state name set is unique, the state name is unique
        // as long as the state set is the same, the state name is the same
        // use the simplest way: concatenate all state names with a separator
        std::string unique_state_name;
        unique_state_name += "[\n";

        // copy the nfa_state_names and sort
        std::vector<std::string> sorted_state_names(nfa_state_names.begin(), nfa_state_names.end());
        std::sort(sorted_state_names.begin(), sorted_state_names.end());
        // iterate through the sorted state names
        for (const auto &state_name : sorted_state_names)
        {
            // add the state name to the unique state name
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

std_nfa_dfa_converter_helper::NFAClosure std_nfa_dfa_converter_helper::get_state_closure(const nfa_model::NFA &nfa, const std::string &state)
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

std_nfa_dfa_converter_helper::NFAClosure std_nfa_dfa_converter_helper::merge_closures(const std::unordered_set<NFAClosure> &closures)
{
    try
    {
        // merge the closures into one closure
        NFAClosure merged_closure;
        merged_closure.has_accepting_state = false;
        merged_closure.has_initial_state = false;
        for (const auto &closure : closures)
        {
            merged_closure.states.insert(closure.states.begin(), closure.states.end());
            merged_closure.has_accepting_state |= closure.has_accepting_state;
            merged_closure.has_initial_state |= closure.has_initial_state;
        }
        // generate the closure name
        merged_closure.closure_name = generate_unique_state_name(merged_closure.states);
        spdlog::debug("Merged closure name: {}", merged_closure.closure_name);
        return merged_closure;
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error merging closures: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

std_nfa_dfa_converter_helper::NFAClosure std_nfa_dfa_converter_helper::get_state_set_closure(const nfa_model::NFA &nfa, const std::unordered_set<std::string> &states)
{
    try
    {
        // get the closure for each state in the set
        std::unordered_set<NFAClosure> closures;
        for (const auto &state : states)
        {
            NFAClosure closure = get_state_closure(nfa, state);
            closures.insert(closure);
        }
        // merge the closures into one closure
        return merge_closures(closures);
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error getting closure for state set: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

std::unordered_set<std_nfa_dfa_converter_helper::NFAClosure>
std_nfa_dfa_converter_helper::generate_closure_set(const nfa_model::NFA &nfa)
{
    try
    {
        spdlog::debug("Generating closure set:");
        std::unordered_set<NFAClosure> closure_set;
        int closure_set_increment = 1;
        // initialize the closure set with the closure of the start state
        NFAClosure start_state_closure = get_state_closure(nfa, nfa.start_state);
        closure_set.insert(start_state_closure);
        spdlog::debug("Initial closure set: {}", start_state_closure.closure_name);
        // start growing the closure set
        int accepting_closure_count = 0;
        int initial_closure_count = 0;
        while (closure_set_increment > 0)
        {
            closure_set_increment = 0;
            std::unordered_set<NFAClosure> new_closures;
            // iterate through the closure set
            for (const auto &closure : closure_set)
            {
                std::unordered_map<std::string, std::unordered_set<std::string>> transition_reachable_states;
                // iterate through the closure states
                for (const auto &state : closure.states)
                {
                    // check if the state has any non-epsilon transitions
                    if (nfa.non_epsilon_transitions.find(state) == nfa.non_epsilon_transitions.end())
                    {
                        spdlog::debug("No non-epsilon transitions for state {}", state);
                        continue;
                    }
                    // iterate through the non-epsilon transitions
                    for (const auto &transition : nfa.non_epsilon_transitions.at(state))
                    {
                        // for now we don't build closure transitions but just find the reachable states
                        std::string input_string = transition.first;
                        std::string next_state = transition.second;
                        // add the next state to the corresponding input string in transition_reachable_states
                        transition_reachable_states[input_string].insert(next_state);
                    }
                }
                // iterate through the transition reachable states
                for (const auto &reachable_states : transition_reachable_states)
                {
                    std::string input_string = reachable_states.first;
                    std::unordered_set<std::string> reachable_state_set = reachable_states.second;
                    spdlog::debug("Closure {} has reachable states for input {}: {}", closure.closure_name, input_string, fmt::join(reachable_state_set, ", "));
                    // get the closure for the reachable states
                    NFAClosure new_closure = get_state_set_closure(nfa, reachable_state_set);
                    // check if the new closure is already in the closure set
                    if (closure_set.find(new_closure) == closure_set.end())
                    {
                        // add the new closure to the new closures set
                        new_closures.insert(new_closure);
                        spdlog::debug("New closure found: {}", new_closure.closure_name);
                        // increment the closure set increment
                        closure_set_increment++;
                        if (new_closure.has_accepting_state)
                        {
                            spdlog::debug("This closure has an accepting state");
                        }
                        if (new_closure.has_initial_state)
                        {
                            spdlog::debug("This closure has the initial state");
                        }
                    }
                    else
                    {
                        spdlog::debug("Omitting one closure as it is already in the closure set");
                    }
                }
            }

            // add the new closures to the closure set
            for (const auto &new_closure : new_closures)
            {
                closure_set.insert(new_closure);
                if (new_closure.has_accepting_state)
                {
                    accepting_closure_count++;
                }
                if (new_closure.has_initial_state)
                {
                    initial_closure_count++;
                    if (initial_closure_count > 1)
                    {
                        std::string error_message = "The closure set has more than one initial state";
                        spdlog::error(error_message);
                        throw std::runtime_error(error_message);
                    }
                }
            }
            spdlog::debug("The closure set has grown by {} closures to {} closures", closure_set_increment, closure_set.size());
        }
        // check if the closure set is empty
        if (closure_set.empty())
        {
            std::string error_message = "The closure set is empty";
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
        spdlog::debug("Closure set generation completed with {} closures, {} of them are accepting closures, and {} of them are initial closures", closure_set.size(), accepting_closure_count, initial_closure_count);
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

NFADFABidirectionalMapping std_nfa_dfa_converter_helper::generate_state_mapping(const std::unordered_set<NFAClosure> &closure_set)
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
                state_mapping.nfa_to_dfa_mapping[nfa_state].insert(dfa_state_name);
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

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> std_nfa_dfa_converter_helper::generate_dfa_transitions(const nfa_model::NFA &nfa, const NFADFABidirectionalMapping &state_mapping, dfa_model::DFA<std::string> &dfa)
{
    try
    {
        spdlog::debug("Generating DFA transitions:");
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> dfa_transitions;
        int generated_transitions_count = 0;
        // iterate through the DFA states
        for (const auto &dfa_state : dfa.states_set)
        {
            // iterate through the symbols in the DFA character set
            for (const auto &symbol : dfa.character_set)
            {
                // get the NFA states for the DFA state
                std::unordered_set<std::string> nfa_states = state_mapping.dfa_to_nfa_mapping.at(dfa_state);
                // mark the nfa states that have a transition for this symbol
                std::unordered_set<std::string> nfa_states_has_transition;
                // the nfa states that are reachable from the current nfa states at this symbol
                std::unordered_set<std::string> reachable_nfa_states;
                // iterate through the NFA states
                for (const auto &nfa_state : nfa_states)
                {
                    // check if the NFA state has a transition for the symbol
                    bool has_transition = false;
                    if (nfa.non_epsilon_transitions.find(nfa_state) != nfa.non_epsilon_transitions.end())
                    {
                        if (nfa.non_epsilon_transitions.at(nfa_state).find(symbol) != nfa.non_epsilon_transitions.at(nfa_state).end())
                        {
                            has_transition = true;
                        }
                    }
                    // if the NFA state has a transition for the symbol
                    if (has_transition)
                    {
                        // add it to the NFA states that have a transition for this symbol
                        nfa_states_has_transition.insert(nfa_state);
                        // get the next state for the symbol
                        std::string next_state = nfa.non_epsilon_transitions.at(nfa_state).at(symbol);
                        // add the next state to the reachable NFA states
                        reachable_nfa_states.insert(next_state);
                        spdlog::debug("DFA state {} has NFA state {} with transition for symbol {} to NFA state {}", dfa_state, nfa_state, symbol, next_state);
                    }
                        
                }
                // now we have the reachable NFA states for the symbol, this set should precisely correspond to a DFA state
                // check if the reachable NFA states are empty
                if (reachable_nfa_states.empty())
                {
                    spdlog::debug("DFA state {} has no reachable NFA states for symbol {}", dfa_state, symbol);
                    continue;
                }
                // get the closure for the reachable NFA states
                NFAClosure reachable_closure = get_state_set_closure(nfa, reachable_nfa_states);
                // check if the reachable closure is already in the DFA states
                if (dfa.states_set.find(reachable_closure.closure_name) != dfa.states_set.end())
                {
                    // add the transition to the DFA transitions
                    dfa_transitions[dfa_state].insert({symbol, reachable_closure.closure_name});
                    spdlog::debug("DFA transition: {} --{}--> {}", dfa_state, symbol, reachable_closure.closure_name);
                    // increment the generated transitions count
                    generated_transitions_count++;
                }
                else
                {
                    // if the reachable closure is not in the DFA states, we need to add it
                    std::string error_message = "DFA state " + dfa_state + " has a transition to a non-existing DFA state " + reachable_closure.closure_name;
                    spdlog::error(error_message);
                    throw std::runtime_error(error_message);
                }
            }
        }
        spdlog::debug("Generated {} DFA transitions", generated_transitions_count);
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