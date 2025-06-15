#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "dfa_model.h"
#include "itemset_generator.h"
#include "lr1_parsing_table_generator.h"
#include "spdlog/spdlog.h"
#include "cfg_analyzer.h"

lr_parsing_model::ItemSet LR1ParsingTableGenerator::generate_item_set(const cfg_model::CFG &cfg)
{
    try
    {
        // Generate the item set
        ItemSetGenerator item_set_generator;
        lr_parsing_model::ItemSet item_set = item_set_generator.generate_item_set(cfg);

        return item_set;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating item set: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::LR1ItemSetDFAGenerationResult LR1ParsingTableGenerator::generate_item_set_dfa(const cfg_model::CFG &cfg)
{
    try
    {
        spdlog::debug("Generating item set DFA for LR(1) parsing table...");
        // preparation: LR(0) item set, blank LR(1) item set, first/follow sets, and an empty DFA
        lr_parsing_model::ItemSet lr0_item_set;
        lr0_item_set = generate_item_set(cfg);
        // get the expanded cfg
        cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);
        // generate the blank LR(1) item set from the LR(0) item set
        lr_parsing_model::ItemSet new_lr1_item_set = lr1_parsing_table_generator_helper::generate_blank_lr1_item_set(lr0_item_set, pool_);
        // generate first and follow sets
        CFGAnalyzer cfg_analyzer(expanded_cfg);
        cfg_analyzer.computeFirstSet();
        cfg_analyzer.computeFollowSet();
        // get the first and follow sets
        const cfg_model::FirstSet &first_set = cfg_analyzer.getFirstSet();
        const cfg_model::FollowSet &follow_set = cfg_analyzer.getFollowSet();
        dfa_model::DFA<std::string> item_set_dfa;
        lr_parsing_model::ItemSetDFAMapping item_set_dfa_mapping;
        // generate the character set for the DFA from the item set symbols
        for (const auto &symbol : new_lr1_item_set.symbol_set)
        {
            // generate a unique name for the DFA character
            std::string dfa_character_name = std::string(symbol);
            // check if the character name already exists
            if (item_set_dfa.character_set.find(dfa_character_name) != item_set_dfa.character_set.end())
            {
                std::string error_msg = "Error: DFA character name already exists: " + dfa_character_name;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            // add the character to the DFA
            item_set_dfa.character_set.insert(dfa_character_name);
            // add the symbol to the mapping
            item_set_dfa_mapping.item_set_symbol_to_dfa_character[symbol] = dfa_character_name;
            item_set_dfa_mapping.dfa_character_to_item_set_symbol[dfa_character_name] = symbol;
            spdlog::debug("Generated CFG-DFA character mapping: {} -> {}", symbol.name, dfa_character_name);
        }

        spdlog::debug("Generating item set DFA from LR(1) item set...");

        // generate the item set DFA
        // generate all closure items
        std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> initial_items;
        initial_items.insert(pool_.get_or_create(std::static_pointer_cast<lr_parsing_model::LR1Item>(new_lr1_item_set.start_item)));
        std::unordered_set<std::string> dfa_states = {};
        auto initial_closure = lr1_parsing_table_generator_helper::grow_closure(initial_items, lr0_item_set, expanded_cfg, first_set, follow_set, pool_);
        // generate the initial DFA state name
        std::string initial_dfa_state_name = lr1_parsing_table_generator_helper::generate_lr1_closure_name(initial_closure);
        // add the initial state to the DFA
        item_set_dfa.states_set.insert(initial_dfa_state_name);
        item_set_dfa.initial_state = initial_dfa_state_name;
        item_set_dfa.accepting_states.insert(initial_dfa_state_name);
        // add the initial closure to the mapping
        for (const auto &item : initial_closure)
        {
            item_set_dfa_mapping.item_set_to_dfa_state[item].insert(initial_dfa_state_name);
            item_set_dfa_mapping.dfa_state_to_item_set[initial_dfa_state_name].insert(item);
        }
        dfa_states.insert(initial_dfa_state_name);
        bool changed = true;
        while (changed)
        {
            changed = false;
            std::vector<std::string> dfa_states_vec(dfa_states.begin(), dfa_states.end());
            for (const auto &dfa_state_name : dfa_states_vec)
            {
                spdlog::debug("Processing DFA state: {}", dfa_state_name);
                // restore the closure
                std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> closure_items;
                if (item_set_dfa_mapping.dfa_state_to_item_set.find(dfa_state_name) != item_set_dfa_mapping.dfa_state_to_item_set.end())
                {
                    auto item_set = item_set_dfa_mapping.dfa_state_to_item_set[dfa_state_name];
                    for (const auto &item : item_set)
                    {
                        closure_items.insert(pool_.get_or_create(std::static_pointer_cast<lr_parsing_model::LR1Item>(item)));
                    }
                }
                else
                {
                    spdlog::warn("DFA state {} not found in mapping, skipping...", dfa_state_name);
                    continue;
                }

                // for each symbol in the item set, generate the next state and grow the closure(if needed)
                for (const auto &symbol : new_lr1_item_set.symbol_set)
                {
                    spdlog::debug("Processing symbol: {}", std::string(symbol));

                    // check if there is a transition for this symbol in the closure items
                    auto generation_items = lr1_parsing_table_generator_helper::generate_initial_closure(
                        closure_items, lr0_item_set, symbol, expanded_cfg, first_set, follow_set, pool_);
                    if (generation_items.empty())
                    {
                        spdlog::debug("No generation items found for symbol: {}", std::string(symbol));
                        continue; // no items can be generated with this symbol
                    }
                    else
                    {
                        auto new_closure = lr1_parsing_table_generator_helper::grow_closure(generation_items, lr0_item_set, expanded_cfg, first_set, follow_set, pool_);

                        // generate a unique name for the new DFA state
                        std::string new_dfa_state_name = lr1_parsing_table_generator_helper::generate_lr1_closure_name(new_closure);
                        spdlog::debug("Generated new DFA state: {}", new_dfa_state_name);
                        // check if the new state already exists
                        if (item_set_dfa.states_set.find(new_dfa_state_name) == item_set_dfa.states_set.end())
                        {
                            // add the new state to the DFA
                            item_set_dfa.states_set.insert(new_dfa_state_name);
                            dfa_states.insert(new_dfa_state_name);
                            spdlog::debug("Added new DFA state: {}", new_dfa_state_name);
                            changed = true; // we have added a new state, so we need to continue processing

                            // add to mapping
                            for (const auto &item : new_closure)
                            {
                                item_set_dfa_mapping.item_set_to_dfa_state[item].insert(new_dfa_state_name);
                                item_set_dfa_mapping.dfa_state_to_item_set[new_dfa_state_name].insert(item);
                            }

                            // iterate over the closure for accepting states
                            if (std::any_of(new_closure.begin(), new_closure.end(),
                                            [](const std::shared_ptr<lr_parsing_model::LR1Item> &item)
                                            {
                                                return item->is_accepting();
                                            }))
                            {
                                item_set_dfa.accepting_states.insert(new_dfa_state_name);
                                spdlog::debug("New DFA state {} is accepting", new_dfa_state_name);
                            }
                            else
                            {
                                spdlog::debug("New DFA state {} is not accepting", new_dfa_state_name);
                            }
                            spdlog::debug("DFA state {} added with {} items", new_dfa_state_name, new_closure.size());
                        }
                        else
                        {
                            spdlog::debug("DFA state {} already exists, skipping...", new_dfa_state_name);
                        }
                        // even if the state already exists, we still need to add the transition
                        // add the transition to the DFA
                        item_set_dfa.add_transition(dfa_state_name, item_set_dfa_mapping.item_set_symbol_to_dfa_character[symbol], new_dfa_state_name);
                    }
                }
            }
        }
        spdlog::debug("Item set DFA generation completed with {} states, {} accepting states, and {} characters",
                      item_set_dfa.states_set.size(), item_set_dfa.accepting_states.size(), item_set_dfa.character_set.size());
        // check the DFA configuration
        dfa_model_helper::check_dfa_configuration<std::string>(item_set_dfa);

        // !!! flush every item in the mapping to the new item set
        // iterate through each DFA state and find the corresponding LR(1) items in the mapping
        for (const auto &dfa_state : item_set_dfa.states_set)
        {
            spdlog::debug("Processing DFA state: {}", dfa_state);
            if (item_set_dfa_mapping.dfa_state_to_item_set.find(dfa_state) != item_set_dfa_mapping.dfa_state_to_item_set.end())
            {
                auto items = item_set_dfa_mapping.dfa_state_to_item_set[dfa_state];
                for (const auto &item : items)
                {
                    // add the item to the new LR(1) item set
                    new_lr1_item_set.items.insert(pool_.get_or_create(std::static_pointer_cast<lr_parsing_model::LR1Item>(item)));
                }
            }
        }
        // find the end symbol
        cfg_model::symbol end_symbol;
        int end_symbol_count = 0;
        for (const auto &symbol : new_lr1_item_set.symbol_set)
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
        // find end item
        int end_item_count = 0;
        for (const auto &item : new_lr1_item_set.items)
        {
            auto lr1_item = std::static_pointer_cast<lr_parsing_model::LR1Item>(item);
            if (lr1_item->left_side_symbol == new_lr1_item_set.start_item->left_side_symbol && lr1_item->sequence_already_parsed.size() == 1 && lr1_item->sequence_to_parse.empty() && lr1_item->lookahead_symbols.size() == 1 && lr1_item->lookahead_symbols.find(end_symbol) != lr1_item->lookahead_symbols.end())
            {
                // this is the end item
                new_lr1_item_set.end_item = pool_.get_or_create(lr1_item);
                end_item_count++;
                spdlog::debug("Found end item: {}", std::string(*new_lr1_item_set.end_item));
            }
        }
        if (end_item_count != 1)
        {
            std::string error_msg = "Error: End item not found or multiple end items found in the LR(1) item set";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }

        // return the result
        lr_parsing_model::LR1ItemSetDFAGenerationResult result;
        result.dfa = item_set_dfa;
        result.item_set_dfa_mapping = item_set_dfa_mapping;
        result.lr1_item_set = new_lr1_item_set;
        spdlog::debug("Returning item set DFA generation result");
        return result;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating item set DFA: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::LRParsingTable LR1ParsingTableGenerator::generate_parsing_table(const cfg_model::CFG &cfg)
{
    try
    {
        spdlog::debug("Generating LR(1) parsing table for CFG: {}", cfg.start_symbol.name);
        // generate the item set DFA
        lr_parsing_model::LR1ItemSetDFAGenerationResult item_set_dfa_result = generate_item_set_dfa(cfg);

        dfa_model::DFA<std::string> dfa = item_set_dfa_result.dfa;
        lr_parsing_model::ItemSetDFAMapping item_set_dfa_mapping = item_set_dfa_result.item_set_dfa_mapping;
        lr_parsing_model::ItemSet lr1_item_set = item_set_dfa_result.lr1_item_set;

        // build the parsing table
        lr_parsing_model::LRParsingTable new_parsing_table;
        // 1. add all symbols from the ItemSet and all states from the DFA to the parsing table
        for (const auto &symbol : lr1_item_set.symbol_set)
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
                    // for terminal symbols, add a shift action
                    if (corresponding_symbol.is_terminal)
                    {
                        lr_parsing_model::Action shift_action;
                        shift_action.action_type = "shift";
                        shift_action.target_state = next_state;
                        // add the action to the parsing table
                        bool added = new_parsing_table.add_action(state, corresponding_symbol, shift_action);
                        if (added)
                            added_shift_actions++;
                    }
                    // for non-terminals, always add a goto state
                    else
                    {
                        // add a goto state
                        bool added = new_parsing_table.add_goto(state, corresponding_symbol, next_state);
                        if (added)
                            added_goto_states++;
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
                // !!! IMPORTANT: THIS IS LR1 ANALYSIS, SO WE NEED TO CHECK THE LOOKAHEAD SYMBOLS
                auto lr1_item = std::static_pointer_cast<lr_parsing_model::LR1Item>(item);
                if (item->sequence_to_parse.empty() && item != lr1_item_set.end_item)
                {
                    // create a reduce action
                    lr_parsing_model::Action reduce_action;
                    reduce_action.action_type = "reduce";
                    reduce_action.reduce_rule_lhs = item->left_side_symbol;
                    reduce_action.reduce_rule_rhs = item->sequence_already_parsed;
                    // add the action to the parsing table for all symbols
                    for (const auto &symbol : lr1_item_set.symbol_set)
                    {
                        // add the action to the parsing if the symbol is a terminal
                        // !!! AND the lookahead symbol is in the lookahead set of the item
                        if (symbol.is_terminal && lr1_item->lookahead_symbols.find(symbol) != lr1_item->lookahead_symbols.end())
                        {
                            // add the action to the parsing table
                            bool added = new_parsing_table.add_action(accepting_state, symbol, reduce_action);
                            if (added)
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
        auto end_item = lr1_item_set.end_item;
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
        else
        {
            end_item_state = *end_item_states.begin();
        }
        // get the end symbol
        cfg_model::symbol end_symbol;
        int end_symbol_count = 0;
        for (const auto &symbol : lr1_item_set.symbol_set)
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
            for (const auto &symbol : lr1_item_set.symbol_set)
            {
                // check if the cell is empty
                if (new_parsing_table.check_cell_empty(state, symbol))
                {
                    // check if the symbol is a terminal or non-terminal
                    if (!symbol.is_terminal)
                    {
                        // fill in the goto state with an empty string
                        std::string next_state = "";
                        bool added = new_parsing_table.add_goto(state, symbol, next_state);
                        if (added)
                            patched_cells++;
                    }
                    else
                    {
                        // fill in the action with an empty action
                        lr_parsing_model::Action empty_action;
                        empty_action.action_type = "empty";
                        bool added = new_parsing_table.add_action(state, symbol, empty_action);
                        if (added)
                            patched_cells++;
                    }
                }
            }
        }
        spdlog::debug("Patched {} empty cells in the parsing table", patched_cells);
        // 6. find the start state and append to the parsing table
        // the start state is the state that corresponds to the start item in the item set
        std::unordered_set<std::string> start_item_states;
        start_item_states = item_set_dfa_mapping.item_set_to_dfa_state.at(lr1_item_set.start_item);
        // there should be only one start item state, otherwise, it's an error
        if (start_item_states.size() != 1)
        {
            std::string error_msg = "Error: Start item state not found or multiple start item states found";
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
        std::string start_item_state = *start_item_states.begin();
        // add the start state to the parsing table
        new_parsing_table.start_state = start_item_state;
        spdlog::debug("Start state of the parsing table: {}", start_item_state);
        // 7. return the parsing table & update the class member variable
        spdlog::debug("Parsing table built successfully, with {} states and {} symbols", new_parsing_table.all_states.size(), new_parsing_table.all_symbols.size());
        for (const auto &state : new_parsing_table.all_states)
        {
            spdlog::debug("State: {}", state);
        }
        for (const auto &symbol : new_parsing_table.all_symbols)
        {
            spdlog::debug("Symbol: {}", symbol.name);
        }

        // Last step: resolve the conflicts in the action table
        // generate itemset-parsing table mapping
        lr_parsing_model::ItemSetParsingTableMapping item_set_parsing_table_mapping;
        item_set_parsing_table_mapping = lr1_parsing_table_generator_helper::generate_item_set_parsing_table_mapping(
            new_parsing_table, lr1_item_set, item_set_dfa_mapping);

        return lr1_parsing_table_generator_helper::resolve_conflicts(new_parsing_table, item_set_parsing_table_mapping);
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating parsing table: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

// helper functions
namespace lr1_parsing_table_generator_helper
{
    lr_parsing_model::ItemSet generate_blank_lr1_item_set(const lr_parsing_model::ItemSet &item_set, LR1ItemPool &pool)
    {
        try
        {
            lr_parsing_model::ItemSet blank_lr1_item_set;
            // get the end symbol
            cfg_model::symbol end_symbol;
            int end_symbol_count = 0;
            for (const auto &symbol : item_set.symbol_set)
            {
                if (symbol.special_property == "END")
                {
                    spdlog::debug("Found end symbol: {}", std::string(symbol));
                    end_symbol = symbol;
                    end_symbol_count++;
                }

                // add the symbol to the blank LR(1) item set
                blank_lr1_item_set.symbol_set.insert(symbol);
            }
            if (end_symbol_count != 1)
            {
                std::string error_msg = "Error: End symbol not found or multiple end symbols found";
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
            auto start_item = pool.get_or_create(*item_set.start_item, end_symbol);
            spdlog::debug("Generated start item: {}", std::string(*start_item));
            auto end_item = pool.get_or_create(*item_set.end_item, end_symbol);
            spdlog::debug("Generated end item: {}", std::string(*end_item));
            blank_lr1_item_set.start_item = start_item;
            blank_lr1_item_set.end_item = end_item;
            blank_lr1_item_set.items.insert(start_item);
            // NOT adding the end item to the items set, this will be handled later and have checking purpose
            // copy the symbol set from the original item set
            blank_lr1_item_set.symbol_set = item_set.symbol_set;
            spdlog::debug("Generated blank LR(1) item set with start item: {}, end item: {}, and {} symbols",
                          std::string(*start_item), std::string(*end_item), blank_lr1_item_set.symbol_set.size());
            return blank_lr1_item_set;
        }
        catch (const std::exception &e)
        {
            std::string error_msg = "Error generating blank LR(1) item set: " + std::string(e.what());
            spdlog::error(error_msg);
            throw std::runtime_error(error_msg);
        }
    }

    // generate a set of LR(1) items from an LR(0) item and their lookahead symbols
    std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> grow_closure(
        const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &initial_items,
        const lr_parsing_model::ItemSet &reference_lr0_item_set,
        const cfg_model::CFG &cfg,
        const cfg_model::FirstSet &first_set,
        const cfg_model::FollowSet &follow_set,
        LR1ItemPool &pool)
    {
        try
        {
            std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> closure_items = initial_items;
            // start the closure process
            bool changed = true;
            while (changed)
            {
                changed = false;
                int previous_size = closure_items.size();
                std::vector<std::shared_ptr<lr_parsing_model::LR1Item>> closure_vec(closure_items.begin(), closure_items.end());
                for (const auto &item : closure_vec)
                {
                    // prepare the lookahead symbols of the items to be generated
                    std::unordered_set<cfg_model::symbol> lookahead_symbols;
                    // 1. if the item is accepting(or complete), or the next symbol is terminal, skip it as it won't generate new items
                    if (item->is_accepting())
                    {
                        continue;
                    }
                    else if (item->sequence_to_parse[0].is_terminal)
                    {
                        continue;
                    }
                    // 2. if the next symbol is the last one in the sequence to parse, inherit the lookahead symbols from the item
                    if (item->sequence_to_parse.size() == 1)
                    {
                        lookahead_symbols = item->lookahead_symbols;
                    }
                    else
                    {
                        cfg_model::symbol next_symbol_to_parse = item->sequence_to_parse[0];
                        cfg_model::symbol lookup_generation_symbol = item->sequence_to_parse[1];

                        // get the first set of lookup_generation_symbol, also check if it has epsilon
                        if (first_set.first_set.find(lookup_generation_symbol) != first_set.first_set.end())
                        {
                            auto first_set_symbols = first_set.first_set.at(lookup_generation_symbol);
                            // if the first set size is 0, check if it has epsilon
                            if (first_set_symbols.size() == 0 && first_set.symbols_with_epsilon.find(lookup_generation_symbol) != first_set.symbols_with_epsilon.end())
                            {
                                // this is a pure epsilon, inherit the lookahead symbols from the item
                                lookahead_symbols = item->lookahead_symbols;
                                spdlog::debug("Inheriting lookahead symbols from item: {}", std::string(*item));
                            }
                            else
                            {
                                // otherwise, add the first set symbols to the lookahead symbols
                                lookahead_symbols.insert(first_set_symbols.begin(), first_set_symbols.end());
                            }
                        }
                    }

                    // 2. generate the next item from the current item
                    std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> next_items = reference_lr0_item_set.get_generation_items(item->sequence_to_parse[0]);

                    // 3. for each next item, create a new LR(1) item with the lookahead symbols and append it to the closure items
                    for (const auto &next_item : next_items)
                    {
                        auto new_lr1_item = pool.get_or_create(*next_item, lookahead_symbols);
                        // check if the new item is already in the closure items
                        if (closure_items.find(new_lr1_item) == closure_items.end())
                        {
                            closure_items.insert(new_lr1_item);
                            spdlog::debug("Added LR(1) item to closure: {}", std::string(*new_lr1_item));
                        }
                    }
                }

                // check if the closure size has changed
                if (closure_items.size() > previous_size)
                {
                    changed = true;
                    spdlog::debug("Closure size changed: {} -> {}", previous_size, closure_items.size());
                }
            }

            spdlog::debug("Closure generation completed, total items: {}", closure_items.size());
            // document the closure items
            spdlog::debug("Closure items:");
            for (const auto &item : closure_items)
            {
                spdlog::debug("{}", std::string(*item));
            }
            return closure_items;
        }
        catch (const std::exception &e)
        {
            std::string error_message = "Error generating closure: ";
            error_message += e.what();
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
    }

    // generate the initial closure items from an existing closure by moving in one symbol
    std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> generate_initial_closure(
        const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &closure_items,
        const lr_parsing_model::ItemSet &reference_lr0_item_set,
        const cfg_model::symbol &next_symbol,
        const cfg_model::CFG &cfg,
        const cfg_model::FirstSet &first_set,
        const cfg_model::FollowSet &follow_set,
        LR1ItemPool &pool)
    {
        try
        {
            std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> initial_closure_items;

            // iterate over the closure items
            for (const auto &item : closure_items)
            {
                // check if the item can generate new items with the next symbol
                if (item->sequence_to_parse.size() > 0 && item->sequence_to_parse[0] == next_symbol)
                {
                    // generate the next item from the current item
                    // move the next symbol from sequence_to_parse to sequence_already_parsed
                    std::vector<cfg_model::symbol> new_sequence_already_parsed = item->sequence_already_parsed;
                    new_sequence_already_parsed.push_back(next_symbol);
                    std::vector<cfg_model::symbol> new_sequence_to_parse(item->sequence_to_parse.begin() + 1, item->sequence_to_parse.end());
                    // find the corresponding LR(0) item
                    std::shared_ptr<lr_parsing_model::Item> next_item;
                    bool found = false;
                    for (const auto &item_set_item : reference_lr0_item_set.items)
                    {
                        if (item_set_item->left_side_symbol == item->left_side_symbol &&
                            item_set_item->sequence_already_parsed == new_sequence_already_parsed &&
                            item_set_item->sequence_to_parse == new_sequence_to_parse)
                        {
                            next_item = item_set_item;
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        spdlog::error("Error: Next item not found in reference item set for symbol: {}", std::string(next_symbol));
                        throw std::runtime_error("Next item not found in reference item set");
                    }
                    auto new_lr1_item = pool.get_or_create(*next_item, item->lookahead_symbols);

                    // add the new LR(1) item to the initial closure items
                    if (initial_closure_items.find(new_lr1_item) == initial_closure_items.end())
                    {
                        initial_closure_items.insert(new_lr1_item);
                        spdlog::debug("Added initial LR(1) item to closure: {}", std::string(*new_lr1_item));
                    }
                }
            }

            spdlog::debug("Initial closure generation completed, total items: {}", initial_closure_items.size());
            if (initial_closure_items.empty())
            {
                spdlog::warn("No initial closure items generated for symbol: {}", std::string(next_symbol));
            }
            else
            {
                spdlog::debug("Initial closure items:");
                for (const auto &item : initial_closure_items)
                {
                    spdlog::debug("{}", std::string(*item));
                }
            }
            // return the initial closure items
            return initial_closure_items;
        }
        catch (const std::exception &e)
        {
            std::string error_message = "Error generating initial closure: ";
            error_message += e.what();
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
    }

    std::string generate_lr1_closure_name(const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &closure_items)
    {
        try
        {
            std::vector<std::string> item_names;
            for (const auto &item : closure_items)
            {
                item_names.push_back(std::string(*item));
            }
            // sort the item names to ensure a consistent order
            std::sort(item_names.begin(), item_names.end());
            // use the simplest way: concatenate all state names with a separator
            std::string unique_state_name;
            unique_state_name += "[\n";
            // iterate through the sorted state names
            for (const auto &state_name : item_names)
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
            std::string error_message = "Error generating LR(1) closure name: ";
            error_message += e.what();
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
    }

    lr_parsing_model::LRParsingTable resolve_conflicts(const lr_parsing_model::LRParsingTable &parsing_table_to_be_resolved, const lr_parsing_model::ItemSetParsingTableMapping &item_set_parsing_table_mapping, const lr1_parsing_table_generator_helper::LR1ConflictResolutionStrategy &strategy)
    {
        try
        {
            spdlog::debug("Resolving conflicts in the parsing table");
            lr_parsing_model::LRParsingTable resolved_parsing_table;
            resolved_parsing_table.all_symbols = parsing_table_to_be_resolved.all_symbols;
            resolved_parsing_table.all_states = parsing_table_to_be_resolved.all_states;
            resolved_parsing_table.action_table = parsing_table_to_be_resolved.action_table;
            resolved_parsing_table.goto_table = parsing_table_to_be_resolved.goto_table;
            resolved_parsing_table.start_state = parsing_table_to_be_resolved.start_state;

            // find all conflicts in the action table
            auto conflicts = resolved_parsing_table.find_conflicts();
            if (conflicts.empty())
            {
                spdlog::debug("No conflicts found in the parsing table");
                return resolved_parsing_table; // no conflicts to resolve
            }

            // iterate through each conflict and resolve it
            for (const auto &conflict : conflicts)
            {
                const std::string &state = conflict.first;
                const cfg_model::symbol &symbol = conflict.second;

                spdlog::warn("Conflict found in state {} for symbol {}: multiple actions", state, std::string(symbol));

                // resolve the conflict by choosing one of the actions
                // this is much easier for LR(1) parsing tables, as we can choose the action based on the lookahead symbol
                std::unordered_set<lr_parsing_model::Action> conflict_actions = resolved_parsing_table.get_actions(state, symbol);
                // erase the conflicting actions from the action table, will fill back later
                resolved_parsing_table.action_table.at(state).erase(symbol);

                // 1. get the items from the item set parsing table mapping for the current state
                auto items = item_set_parsing_table_mapping.parsing_table_cell_to_item_set.at(state).at(symbol);
                spdlog::debug("Found {} items for state {} and symbol {}", items.size(), state, std::string(symbol));
                // 2. categorize the items into shift and reduce actions
                std::vector<std::shared_ptr<lr_parsing_model::LR1Item>> shift_items;
                std::vector<std::shared_ptr<lr_parsing_model::LR1Item>> reduce_items;
                for (const auto &item : items)
                {
                    // check the action type
                    if (!item->is_accepting())
                    {
                        shift_items.push_back(std::static_pointer_cast<lr_parsing_model::LR1Item>(item));
                    }
                    else
                    {
                        reduce_items.push_back(std::static_pointer_cast<lr_parsing_model::LR1Item>(item));
                    }
                }
                // 3. first process reduce action items
                // this will remove the not-reducable items, solving reduce-reduce conflicts and also preparing for shift-reduce conflicts
                if (reduce_items.size() == 0)
                {
                    spdlog::debug("No reduce items found in state {} for symbol {}, all actions are shift actions", state, std::string(symbol));
                }
                else
                {
                    // remove all reduce item whose lookahead symbols does not contain the current symbol
                    std::vector<std::shared_ptr<lr_parsing_model::LR1Item>> reduce_items_to_erase;
                    std::vector<lr_parsing_model::Action> reduce_actions_to_erase;
                    for (const auto &reduce_item : reduce_items)
                    {
                        if (reduce_item->lookahead_symbols.find(symbol) == reduce_item->lookahead_symbols.end())
                        {
                            spdlog::debug("Removing reduce item {} from state {} for symbol {}: lookahead symbols do not contain the current symbol",
                                          std::string(*reduce_item), state, std::string(symbol));
                            reduce_items_to_erase.push_back(reduce_item);
                            // also find the corresponding action in the conflict actions
                            for (const auto &action : conflict_actions)
                            {
                                if (action.action_type == "reduce" && action.reduce_rule_lhs == reduce_item->left_side_symbol &&
                                    action.reduce_rule_rhs == reduce_item->sequence_already_parsed)
                                {
                                    spdlog::debug("Removing action of item {} from state {} for symbol {}: it corresponds to the reduce item being removed",
                                                  std::string(*reduce_item), state, std::string(symbol));
                                    reduce_actions_to_erase.push_back(action);
                                }
                            }
                        }
                    }
                    // erase the items from the reduce items & corresponding actions
                    for (const auto &reduce_item : reduce_items_to_erase)
                    {
                        reduce_items.erase(std::remove(reduce_items.begin(), reduce_items.end(), reduce_item), reduce_items.end());
                    }
                    // erase the actions from the conflict actions
                    for (const auto &reduce_action : reduce_actions_to_erase)
                    {
                        conflict_actions.erase(reduce_action);
                    }
                    // there should be at most one reduce item left
                    if (reduce_items.size() > 1)
                    {
                        std::string error_message = "Error: Multiple reduce items left after resolving reduce-reduce conflict in state " + state + " for symbol " + std::string(symbol);
                        spdlog::error(error_message);
                        throw std::runtime_error(error_message);
                    }
                    else
                    {
                        spdlog::debug("RR conflict resolved in state {} for symbol {}: {}", state, std::string(symbol), std::string(*reduce_items[0]));
                    }
                }
                // 4. then resolve shift-reduce conflicts
                // check if there's any reduce item left
                if (reduce_items.empty())
                {
                    // conflict resolved successfully for this state and symbol
                    spdlog::debug("Conflict resolved in state {} for symbol {}: no reduce items left", state, std::string(symbol));
                    // add all shift actions back to the action table
                    for (const auto &action : conflict_actions)
                    {
                        resolved_parsing_table.add_action(state, symbol, action);
                    }
                }
                else
                {
                    // now there is exactly one reduce item left
                    // for LR1 analysis, the requirement is that no shift item should present in the same state for the same symbol
                    if (shift_items.empty())
                    {
                        // add all(the only one) reduce action back to the action table
                        for (const auto &action : conflict_actions)
                        {
                            resolved_parsing_table.add_action(state, symbol, action);
                        }
                    }
                    else
                    {
                        // this conflict is not resolvable for LR(1) parsing
                        // document conflict details and throw a warning, then solve using the strategy
                        spdlog::warn("Shift-reduce conflict found in state {} for symbol {}: multiple actions", state, std::string(symbol));
                        spdlog::warn("Shift items: ");
                        for (const auto &shift_item : shift_items)
                        {
                            spdlog::warn("  {}", std::string(*shift_item));
                        }
                        spdlog::warn("Reduce item: {}", std::string(*reduce_items[0]));
                        // resolve the conflict based on the strategy
                        if (strategy == LR1ConflictResolutionStrategy::SHIFT_OVER_REDUCE)
                        {
                            // prefer shift action, so we will keep the shift actions and discard the reduce action
                            spdlog::debug("Resolving conflict by preferring shift action in state {} for symbol {}", state, std::string(symbol));
                            for (const auto &action : conflict_actions)
                            {
                                if (action.action_type == "shift")
                                {
                                    resolved_parsing_table.add_action(state, symbol, action);
                                }
                            }
                        }
                        else if (strategy == LR1ConflictResolutionStrategy::REDUCE_OVER_SHIFT)
                        {
                            // prefer reduce action, so we will keep the reduce action and discard the shift actions
                            spdlog::debug("Resolving conflict by preferring reduce action in state {} for symbol {}", state, std::string(symbol));
                            for (const auto &action : conflict_actions)
                            {
                                if (action.action_type == "reduce")
                                {
                                    resolved_parsing_table.add_action(state, symbol, action);
                                }
                            }
                        }
                        else
                        {
                            std::string error_message = "Error: Unknown conflict resolution strategy";
                            spdlog::error(error_message);
                            throw std::runtime_error(error_message);
                        }
                        spdlog::debug("Conflict resolved in state {} for symbol {}: using strategy {}", state, std::string(symbol), strategy == LR1ConflictResolutionStrategy::SHIFT_OVER_REDUCE ? "PREFER_SHIFT" : "PREFER_REDUCE");
                    }
                }
            }

            spdlog::debug("Conflicts resolved successfully in the parsing table");
            // return the resolved parsing table
            return resolved_parsing_table;
        }
        catch (const std::exception &e)
        {
            std::string error_message = "Error resolving conflicts in the parsing table: ";
            error_message += e.what();
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
    }

    lr_parsing_model::ItemSetParsingTableMapping generate_item_set_parsing_table_mapping(
        const lr_parsing_model::LRParsingTable &parsing_table,
        const lr_parsing_model::ItemSet &item_set,
        const lr_parsing_model::ItemSetDFAMapping &item_set_dfa_mapping)
    {
        try
        {

            // create the mapping
            // this is basically a the same as the itemset to dfa mapping, but for each mapping, add the symbol to match the parsing table cells
            lr_parsing_model::ItemSetParsingTableMapping new_item_set_parsing_table_mapping;
            // iterate over the ItemSet items
            for (const auto &item : item_set.items)
            {
                // find the corresponding DFA states
                std::unordered_set<std::string> dfa_state_names = item_set_dfa_mapping.item_set_to_dfa_state.at(item);
                // iterate over the DFA states
                for (const auto &dfa_state_name : dfa_state_names)
                {
                    // find the corresponding parsing table cells
                    // iterate over all symbols in the parsing table
                    for (const auto &symbol : parsing_table.all_symbols)
                    {
                        // check if the cell is empty
                        if (!parsing_table.check_cell_empty(dfa_state_name, symbol))
                        {
                            // add the mapping to the new mapping
                            new_item_set_parsing_table_mapping.parsing_table_cell_to_item_set[dfa_state_name][symbol].insert(item);
                            new_item_set_parsing_table_mapping.item_set_to_parsing_table_cell[item].insert({dfa_state_name, symbol});
                        }
                    }
                }
            }
            // update the class member variable
            new_item_set_parsing_table_mapping;
            spdlog::debug("ItemSet parsing table mapping generated successfully");
            return new_item_set_parsing_table_mapping;
        }
        catch (const std::exception &e)
        {
            std::string error_message = "Error generating item set parsing table mapping: ";
            error_message += e.what();
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
    }

}