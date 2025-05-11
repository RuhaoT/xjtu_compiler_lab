#include "lr_parsing_model.h"
#include "itemset_to_parsing_table.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// constructor
ItemSetToParsingTable::ItemSetToParsingTable(const lr_parsing_model::ItemSet &item_set)
{
    this->item_set = item_set;
    spdlog::debug("ItemSetToParsingTable constructor called");
    // generate nfa for the item set
    try
    {
        generate_nfa();
        generate_dfa();
        build_parsing_table();
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during ItemSetToParsingTable construction: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::LRParsingTable ItemSetToParsingTable::build_parsing_table()
{
    try
    {
        // check for prerequisites: itemset, nfa, dfa, and their mappings
        if (item_set.items.empty())
        {
            std::string error_msg = "Error: Item set is empty";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        if (nfa.states.empty())
        {
            std::string error_msg = "Error: NFA is empty";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        if (dfa.states_set.empty())
        {
            std::string error_msg = "Error: DFA is empty";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        if (item_set_nfa_mapping.item_set_to_nfa_state.empty())
        {
            std::string error_msg = "Error: Item set to NFA state mapping is empty";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        if (item_set_dfa_mapping.item_set_to_dfa_state.empty())
        {
            std::string error_msg = "Error: Item set to DFA state mapping is empty";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }

        // build the parsing table
        lr_parsing_model::LRParsingTable new_parsing_table;
        // 1. add all symbols from the ItemSet and all states from the DFA to the parsing table
        for (const auto &symbol : item_set.symbol_set)
        {
            new_parsing_table.all_symbols.insert(symbol);
        }
        for (const auto &state : dfa.states_set)
        {
            new_parsing_table.all_states.insert(state);
        }
        // 2. find all shift actions and goto states
        int added_shift_actions = 0;
        int added_goto_states = 0;
        for (const auto &state_transitions : dfa.transitions)
        {
            const std::string &state = state_transitions.first;
            const auto &transitions = state_transitions.second;
            // iterate over all transitions for the current state
            for (const auto &transition : transitions)
            {

                std::string input_char = transition.first;
                std::string next_state = transition.second;
                spdlog::debug("Processing transition: {} --{}--> {}", state, input_char, next_state);
                cfg_model::symbol corresponding_symbol = item_set_dfa_mapping.dfa_character_to_item_set_symbol.at(input_char);
                // get all the items in the next state
                auto items_in_next_state = item_set_dfa_mapping.dfa_state_to_item_set.at(next_state);
                spdlog::debug("Item number in next state for this transition: {}", items_in_next_state.size());
                // iterate over all items in the next state
                for (const auto &item : items_in_next_state)
                {
                    // for terminal symbols, add a shift action as long as the item is not the end item
                    if (corresponding_symbol.is_terminal)
                    {
                        if (item == item_set.end_item)
                        {
                            // skip the end item and accepting states
                            continue;
                        }
                        lr_parsing_model::Action shift_action;
                        shift_action.action_type = "shift";
                        shift_action.target_state = next_state;
                        // add the action to the parsing table
                        bool added = new_parsing_table.add_action(state, corresponding_symbol, shift_action);
                        if (added) added_shift_actions++;
                    }
                    // for non-terminals, always add a goto state
                    else
                    {
                        // add a goto state
                        bool added = new_parsing_table.add_goto(state, corresponding_symbol, next_state);
                        if (added) added_goto_states++;
                    }
                }
            }
        }
        spdlog::debug("Added {} shift actions and {} goto states to the parsing table", added_shift_actions, added_goto_states);
        // 3. find all reduce actions
        // iterate over all accepting states in the DFA
        int added_reduce_actions = 0;
        for (const auto &accepting_state : dfa.accepting_states)
        {
            // get all items in the accepting state
            auto items_in_accepting_state = item_set_dfa_mapping.dfa_state_to_item_set.at(accepting_state);
            // iterate over all items in the accepting state
            for (const auto &item : items_in_accepting_state)
            {
                // check if the item is an accepting state and not the end item
                if (item->sequence_to_parse.empty() && item != item_set.end_item)
                {
                    // create a reduce action
                    lr_parsing_model::Action reduce_action;
                    reduce_action.action_type = "reduce";
                    reduce_action.reduce_rule_lhs = item->left_side_symbol;
                    reduce_action.reduce_rule_rhs = item->sequence_already_parsed;
                    // add the action to the parsing table for all symbols
                    for (const auto &symbol : item_set.symbol_set)
                    {
                        // add the action to the parsing if the symbol is a terminal
                        if (symbol.is_terminal)
                        {
                            // add the action to the parsing table
                            bool added = new_parsing_table.add_action(accepting_state, symbol, reduce_action);
                            if(added)
                            {
                                added_reduce_actions++;
                            }
                        }
                    }
                }
            }
        }
        spdlog::debug("Added {} reduce actions to the parsing table", added_reduce_actions);
        // 4. find the accept action
        // get the end item from the item set and find the corresponding state
        auto end_item = item_set.end_item;
        std::unordered_set<std::string> end_item_states;
        end_item_states = item_set_dfa_mapping.item_set_to_dfa_state.at(end_item);
        std::string end_item_state = "";
        // there should be only one end item state
        if (end_item_states.size() != 1)
        {
            std::string error_msg = "Error: End item state not found or multiple end item states found";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        else{
            end_item_state = *end_item_states.begin();
        }
        // get the end symbol
        cfg_model::symbol end_symbol;
        int end_symbol_count = 0;
        for (const auto &symbol: item_set.symbol_set)
        {
            if (symbol.special_property == "END")
            {
                spdlog::debug("Found end symbol: {}", std::string(symbol));
                end_symbol = symbol;
                end_symbol_count++;
            }
        }
        if (end_symbol_count != 1)
        {
            std::string error_msg = "Error: End symbol not found or multiple end symbols found";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        // create an accept action
        lr_parsing_model::Action accept_action;
        accept_action.action_type = "accept";
        new_parsing_table.add_action(end_item_state, end_symbol, accept_action);
        spdlog::debug("Added accept action for state {} and symbol {}", end_item_state, std::string(end_symbol));
        // 5. fill all the empty cells in the parsing table with empty actions
        int patched_cells = 0;
        for (const auto &state : dfa.states_set)
        {
            for (const auto &symbol : item_set.symbol_set)
            {
                // check if the cell is empty
                if (new_parsing_table.check_cell_empty(state,symbol))
                {
                    // check if the symbol is a terminal or non-terminal
                    if (!symbol.is_terminal)
                    {
                        // fill in the goto state with an empty string
                        std::string next_state = "";
                        bool added = new_parsing_table.add_goto(state, symbol, next_state);
                        if (added) patched_cells++;
                    }
                    else
                    {
                        // fill in the action with an empty action
                        lr_parsing_model::Action empty_action;
                        empty_action.action_type = "empty";
                        bool added = new_parsing_table.add_action(state, symbol, empty_action);
                        if (added) patched_cells++;
                    }
                }
            }
        }
        spdlog::debug("Patched {} empty cells in the parsing table", patched_cells);
        // 6. return the parsing table & update the class member variable
        spdlog::debug("Parsing table built successfully, with {} states and {} symbols", new_parsing_table.all_states.size(), new_parsing_table.all_symbols.size());
        for (const auto &state : new_parsing_table.all_states)
        {
            spdlog::debug("State: {}", state);
        }
        for (const auto &symbol : new_parsing_table.all_symbols)
        {
            spdlog::debug("Symbol: {}", symbol.name);
        }
        parsing_table = new_parsing_table;
        return parsing_table;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during parsing table building: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

// generate nfa for the item set
lr_parsing_model::ItemSetNFAGenerationResult ItemSetToParsingTable::generate_nfa()
{
    try
    {
        nfa_model::NFA new_nfa;
        lr_parsing_model::ItemSetNFAMapping new_item_set_nfa_mapping;
        // 1. generate NFA character set
        for (const auto &symbol : item_set.symbol_set)
        {
            // generate a unique name for the NFA character
            std::string character_name = itemset_to_parsing_table_helper::generate_nfa_character_name(symbol);
            // check if the character name already exists
            if (new_nfa.character_set.find(character_name) != new_nfa.character_set.end())
            {
                std::string error_msg = "Error: NFA character name already exists: " + character_name;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            // add the character to the NFA
            new_nfa.character_set.insert(character_name);
            // add the symbol to the mapping
            new_item_set_nfa_mapping.item_set_symbol_to_nfa_character[symbol] = character_name;
            new_item_set_nfa_mapping.nfa_character_to_item_set_symbol[character_name] = symbol;
            spdlog::debug("Generated CFG-NFA character mapping: {} -> {}", symbol.name, character_name);
        }
        // 2. generate NFA states
        for (const auto &item : item_set.items)
        {
            std::string state_name = itemset_to_parsing_table_helper::generate_nfa_state_name(*item);
            // check if the state name already exists
            if (new_nfa.states.find(state_name) != new_nfa.states.end())
            {
                std::string error_msg = "Error: NFA state name already exists: " + state_name;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            // add the state to the NFA
            new_nfa.states.insert(state_name);
            // add the item to the mapping
            new_item_set_nfa_mapping.item_set_to_nfa_state[item] = state_name;
            new_item_set_nfa_mapping.nfa_state_to_item_set[state_name] = item;
            // check if the item is an accepting state
            if (item->sequence_to_parse.empty())
            {
                new_nfa.accepting_states.insert(state_name);
            }
            // check if the item is the start item
            if (item == item_set.start_item)
            {
                new_nfa.start_state = state_name;
            }
        }
        // 3. generate NFA transitions
        // iterate over all items in the item set
        for (const auto &item : item_set.items)
        {
            // check if the sequence to parse is empty, if so, skip
            if (item->sequence_to_parse.empty())
            {
                continue;
            }
            // get the first symbol in the sequence to parse
            const auto &first_symbol = item->sequence_to_parse[0];
            // be it a terminal or non-terminal, a transition to the next parsing item is needed
            // get the NFA state name in the mapping
            std::string from_state = new_item_set_nfa_mapping.item_set_to_nfa_state[item];
            // get the NFA character name in the mapping
            std::string character_name = new_item_set_nfa_mapping.item_set_symbol_to_nfa_character[first_symbol];
            // construct the next parsing item
            lr_parsing_model::Item next_item;
            next_item.left_side_symbol = item->left_side_symbol;
            next_item.sequence_already_parsed = item->sequence_already_parsed;
            next_item.sequence_already_parsed.push_back(first_symbol);
            next_item.sequence_to_parse = std::vector<cfg_model::symbol>(item->sequence_to_parse.begin() + 1, item->sequence_to_parse.end());
            // get the NFA state name for the next item
            std::string to_state = itemset_to_parsing_table_helper::generate_nfa_state_name(next_item);
            // ensure the name is already in the NFA
            if (new_nfa.states.find(to_state) == new_nfa.states.end())
            {
                std::string error_msg = "Error: NFA state name not found: " + to_state;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            // add the transition to the NFA
            new_nfa.non_epsilon_transitions[from_state][character_name] = to_state;
            spdlog::debug("Generated NFA transition: {} --{}--> {}", from_state, character_name, to_state);
            // if the first symbol is not a terminal, an extra set of epsilon transitions is needed
            if (!first_symbol.is_terminal)
            {
                // the first symbol is a non-terminal, generate epsilon transitions
                // find all items that has the first symbol as the left side symbol & all the sequence have not been parsed
                for (const auto &next_item : item_set.items)
                {
                    // check if the left side symbol is the same as the first symbol
                    if (next_item->left_side_symbol == first_symbol && next_item->sequence_already_parsed.empty())
                    {
                        // get the NFA state name in the mapping
                        std::string from_state = new_item_set_nfa_mapping.item_set_to_nfa_state[item];
                        // get the NFA state name for the next item
                        std::string to_state = new_item_set_nfa_mapping.item_set_to_nfa_state[next_item];
                        // add the epsilon transition to the NFA
                        new_nfa.epsilon_transitions.insert({from_state, to_state});
                        spdlog::debug("Generated NFA epsilon transition: {} --ε--> {}", from_state, to_state);
                        spdlog::debug("Now the NFA has {} epsilon transitions", new_nfa.epsilon_transitions.size());
                    }
                }
            }
        }
        spdlog::debug("NFA generation completed with {} states, {} accepting states, {} characters, {} non-epsilon transitions, and {} epsilon transitions",
                      new_nfa.states.size(), new_nfa.accepting_states.size(), new_nfa.character_set.size(), new_nfa.count_non_epsilon_transitions(), new_nfa.epsilon_transitions.size());
        lr_parsing_model::ItemSetNFAGenerationResult result;
        // update the class member variables
        nfa = new_nfa;
        item_set_nfa_mapping = new_item_set_nfa_mapping;
        spdlog::debug("Updated NFA and mapping in the class");
        result.nfa = nfa;
        result.item_set_nfa_mapping = item_set_nfa_mapping;
        return result;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during ItemSet NFA generation: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::ItemSetDFAGenerationResult ItemSetToParsingTable::generate_dfa()
{
    try
    {
        // check if the NFA is generated
        if (nfa.states.empty())
        {
            std::string error_msg = "NFA is not generated yet. Please generate NFA first.";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        // convert the NFA to DFA
        StandardNFA_DFA_Converter nfa_dfa_converter;
        NFACTDFAConvertionResult conversion_result = nfa_dfa_converter.convert_nfa_to_dfa(nfa);

        // create mapping from ItemSet
        lr_parsing_model::ItemSetDFAMapping new_item_set_dfa_mapping;
        // iterate over the ItemSet items
        for (const auto &item : item_set.items)
        {
            // get the NFA state name in the mapping
            std::string nfa_state_name = item_set_nfa_mapping.item_set_to_nfa_state[item];
            // get the DFA state name in the conversion result
            std::unordered_set<std::string> dfa_state_names = conversion_result.state_mapping.nfa_to_dfa_mapping[nfa_state_name];
            // add the mapping to the new mapping
            new_item_set_dfa_mapping.item_set_to_dfa_state[item].insert(dfa_state_names.begin(), dfa_state_names.end());
            // for each DFA state, add the mapping to the ItemSet
            for (const auto &dfa_state_name : dfa_state_names)
            {
                new_item_set_dfa_mapping.dfa_state_to_item_set[dfa_state_name].insert(item);
            }
        }
        // iterate over the ItemSet symbols
        for (const auto &symbol : item_set.symbol_set)
        {
            // the NFA and DFA character set are the same
            // so a simple check is enough
            // get the NFA character name in the mapping
            std::string nfa_character_name = item_set_nfa_mapping.item_set_symbol_to_nfa_character[symbol];
            // check if the character name exists in the DFA character set
            if (conversion_result.dfa.character_set.find(nfa_character_name) == conversion_result.dfa.character_set.end())
            {
                std::string error_msg = "Error: DFA character name not found: " + nfa_character_name;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            // add the mapping to the new mapping
            new_item_set_dfa_mapping.item_set_symbol_to_dfa_character[symbol] = nfa_character_name;
            new_item_set_dfa_mapping.dfa_character_to_item_set_symbol[nfa_character_name] = symbol;
        }
        // update the class member variables
        dfa = conversion_result.dfa;
        item_set_dfa_mapping = new_item_set_dfa_mapping;
        spdlog::debug("ItemSet DFA generation completed");
        dfa_model_helper::check_conflict_tolerant_dfa_configuration<std::string>(dfa);
        return {dfa, item_set_dfa_mapping};
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during ItemSet DFA generation: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

// helper functions
std::string itemset_to_parsing_table_helper::generate_nfa_state_name(const lr_parsing_model::Item &item)
{
    // the objective is to generate a name for the NFA state
    // the name should be unique as long as the item is unique
    // for now we just use the string representation of the item
    std::string state_name = item;
    spdlog::debug("Generated NFA state name: {}", state_name);
    return state_name;
}

std::string itemset_to_parsing_table_helper::generate_nfa_character_name(const cfg_model::symbol &symbol)
{
    // the objective is to generate a name for the NFA character
    // the name should be unique as long as the symbol is unique
    // for now we just use the string representation of the symbol
    std::string character_name = symbol.name;
    spdlog::debug("Generated NFA character name: {}", character_name);
    return character_name;
}