#include "cfg_model.h"
#include "cfg_analyzer.h"
#include "lr_parsing_model.h"
#include "slr1_conflict_resolver.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

SLR1ConflictResolver::SLR1ConflictResolver(const lr_parsing_model::LRParsingTable &parsing_table, const cfg_model::CFG &cfg, const lr_parsing_model::ItemSetParsingTableMapping &item_set_parsing_table_mapping)
    : parsing_table(parsing_table), cfg(cfg), item_set_parsing_table_mapping(item_set_parsing_table_mapping)
{
    // initialize the resolved parsing table to empty
    resolved_parsing_table = lr_parsing_model::LRParsingTable();
}

bool SLR1ConflictResolver::resolve_conflicts()
{
    try
    {
        spdlog::debug("Resolving conflicts in the parsing table...");
        // copy the parsing table to the resolved parsing table
        resolved_parsing_table.all_symbols = parsing_table.all_symbols;
        resolved_parsing_table.all_states = parsing_table.all_states;
        resolved_parsing_table.action_table = parsing_table.action_table;
        resolved_parsing_table.goto_table = parsing_table.goto_table;
        resolved_parsing_table.start_state = parsing_table.start_state;
        // get the conflicts in the parsing table
        auto conflicts = parsing_table.find_conflicts();
        // check if there are any conflicts
        if (conflicts.empty())
        {
            spdlog::debug("No conflicts found in the parsing table");
            return true; // no conflicts to resolve
        }
        else
        {
            spdlog::debug("Found {} conflicts in the parsing table", conflicts.size());
        }
        // conflict solving preparation: find the follow sets
        CFGAnalyzer cfg_analyzer(cfg);
        cfg_analyzer.computeFirstSet();
        cfg_analyzer.computeFollowSet();
        const cfg_model::FollowSet &follow_set = cfg_analyzer.getFollowSet();
        // iterate over the conflicts and resolve them
        for (const auto &conflict : conflicts)
        {
            const std::string &state = conflict.first;
            const cfg_model::symbol &symbol = conflict.second;
            spdlog::debug("Resolving conflict for state {} and symbol {}", state, std::string(symbol));
            // get the items in the conflict from mapping
            std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> items_in_conflict = item_set_parsing_table_mapping.parsing_table_cell_to_item_set[state][symbol];
            spdlog::debug("Found {} items in the conflict", items_in_conflict.size());
            // get the shift symbols from the items
            std::unordered_set<cfg_model::symbol> shift_symbols = slr1_conflict_resolver_helper::get_shift_symbols(items_in_conflict);
            int shift_symbols_count = shift_symbols.size();
            spdlog::debug("Found {} shift symbols in the conflict", shift_symbols_count);
            // for clarity we iterate over the items two times, one for checking the conflict and one for resolving it
            // check if the conflict is resolvable by checking the intersection of the shift symbols and the follow sets of accepting items
            std::unordered_set<cfg_model::symbol> total_symbols = shift_symbols;
            int expected_total_symbols_count = shift_symbols.size();
            for (const auto &item : items_in_conflict)
            {
                // check if the item is accepting
                if (item->is_accepting())
                {
                    // get the follow set of the item
                    std::unordered_set<cfg_model::symbol> current_item_follow_set = follow_set.follow_set.at(item->left_side_symbol);
                    // add the follow set to the total symbols
                    total_symbols.insert(current_item_follow_set.begin(), current_item_follow_set.end());
                    expected_total_symbols_count += current_item_follow_set.size();
                }
            }
            // check if the shift symbols and the follow sets intersect
            if (expected_total_symbols_count != total_symbols.size())
            {
                // there is a conflict that cannot be resolved
                spdlog::info("Conflict at state {} and symbol {} cannot be resolved because the shift symbols and follow sets intersect", state, std::string(symbol));
                // document the shift symbols and follow sets for debugging
                std::string shift_symbols_str = "Shift symbols: ";
                for (const auto &shift_symbol : shift_symbols)
                {
                    shift_symbols_str += std::string(shift_symbol) + ", ";
                }
                spdlog::debug(shift_symbols_str);
                // document follow set one by one
                for (const auto &item : items_in_conflict)
                {
                    if (item->is_accepting())
                    {
                        std::unordered_set<cfg_model::symbol> current_item_follow_set = follow_set.follow_set.at(item->left_side_symbol);
                        std::string follow_set_str = "Follow set for item " + std::string(item->left_side_symbol) + ": ";
                        for (const auto &follow_symbol : current_item_follow_set)
                        {
                            follow_set_str += std::string(follow_symbol) + ", ";
                        }
                        spdlog::debug(follow_set_str);
                    }
                }
                return false;
            }
            // start resolving the conflict
            // get all potential actions in the conflict
            std::unordered_set<lr_parsing_model::Action> actions_in_conflict = resolved_parsing_table.get_actions(state, symbol);
            // clear the action table cell
            resolved_parsing_table.action_table[state].erase(symbol);
            // 1. check if the symbol is in the shift symbols
            bool resolved_by_shift = false;
            if (shift_symbols.find(symbol) != shift_symbols.end())
            {
                // find the shift action in the conflict
                bool shift_action_found = false;
                for (const auto &action : actions_in_conflict)
                {
                    if (action.action_type == "shift")
                    {
                        // add the shift action to the parsing table
                        resolved_parsing_table.add_action(state, symbol, action);
                        shift_action_found = true;
                        spdlog::debug("Added shift action for state {} and symbol {}", state, std::string(symbol));
                        break;
                    }
                }
                if (!shift_action_found)
                {
                    // no shift action found, this is an error
                    std::string error_message = "Error: No shift action found for state " + state + " and symbol " + std::string(symbol);
                    spdlog::error(error_message);
                    throw std::runtime_error(error_message);
                }
                resolved_by_shift = true;
            }
            if (resolved_by_shift)
            {
                spdlog::debug("Resolved conflict by shift for state {} and symbol {}", state, std::string(symbol));
                continue; // continue to the next conflict
            }
            // 2. if not resolved by shift, find the accepting item whose follow set contains the symbol
            for (const auto &item : items_in_conflict)
            {
                // check if the item is accepting
                if (item->is_accepting())
                {
                    // get the follow set of the item
                    std::unordered_set<cfg_model::symbol> current_item_follow_set = follow_set.follow_set.at(item->left_side_symbol);
                    // check if the symbol is in the follow set
                    if (current_item_follow_set.find(symbol) != current_item_follow_set.end())
                    {
                        // create a reduce action
                        lr_parsing_model::Action reduce_action;
                        reduce_action.action_type = "reduce";
                        reduce_action.reduce_rule_lhs = item->left_side_symbol;
                        reduce_action.reduce_rule_rhs = item->sequence_already_parsed;
                        // add the reduce action to the parsing table
                        resolved_parsing_table.add_action(state, symbol, reduce_action);
                        spdlog::debug("Added reduce action for state {} and symbol {}", state, std::string(symbol));
                    }
                }
            }
        }
        spdlog::debug("Conflicts resolved successfully");
        return true; // all conflicts resolved
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error resolving conflicts: " + std::string(e.what());
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

lr_parsing_model::LRParsingTable SLR1ConflictResolver::get_solved_parsing_table() const
{
    // check if the conflicts are resolved
    if (resolved_parsing_table.action_table.empty())
    {
        std::string error_message = "Error: Conflicts are not resolved yet";
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
    return resolved_parsing_table;
}

// helper functions
std::unordered_set<cfg_model::symbol> slr1_conflict_resolver_helper::get_shift_symbols(const std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> &items)
{
    std::unordered_set<cfg_model::symbol> shift_symbols;
    for (const auto &item : items)
    {
        // check if the item is not accepting
        if (!item->is_accepting())
        {
            // get the next symbol to parse
            cfg_model::symbol next_symbol = item->sequence_to_parse[0];
            shift_symbols.insert(next_symbol);
        }
    }
    return shift_symbols;
}