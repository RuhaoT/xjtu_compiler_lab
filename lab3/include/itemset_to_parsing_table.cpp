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
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during ItemSetToParsingTable construction: " + std::string(e.what());
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
            std::string dfa_state_name = conversion_result.state_mapping.nfa_to_dfa_mapping[nfa_state_name];
            // add the mapping to the new mapping
            new_item_set_dfa_mapping.item_set_to_dfa_state[item] = dfa_state_name;
            new_item_set_dfa_mapping.dfa_state_to_item_set[dfa_state_name].insert(item);
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