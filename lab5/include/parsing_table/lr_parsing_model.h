#ifndef LR_PARSING_MODEL_H
#define LR_PARSING_MODEL_H

#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace lr_parsing_model
{
    struct Item
    {
        cfg_model::symbol left_side_symbol;
        std::vector<cfg_model::symbol> sequence_already_parsed;
        std::vector<cfg_model::symbol> sequence_to_parse;

        // overload the equality operator
        bool operator==(const Item &other) const
        {
            return left_side_symbol == other.left_side_symbol &&
                   sequence_already_parsed == other.sequence_already_parsed &&
                   sequence_to_parse == other.sequence_to_parse;
        }

        // string representation
        operator std::string() const;

        // helper functions
        bool is_accepting() const
        {
            return sequence_to_parse.empty();
        }

        bool is_complete() const
        {
            return sequence_already_parsed.empty() && sequence_to_parse.empty();
        }
    };

} // namespace lr_parsing_model

// Specialization of std::hash for lr_parsing_model::Item
namespace std
{
    template <>
    struct hash<lr_parsing_model::Item>
    {
        size_t operator()(const lr_parsing_model::Item &item) const;
    };
} // namespace std

namespace lr_parsing_model
{
    struct ItemSet
    {
        std::unordered_set<std::shared_ptr<Item>> items;
        std::shared_ptr<Item> start_item;
        std::shared_ptr<Item> end_item;
        std::unordered_set<cfg_model::symbol> symbol_set;
    };

    struct ItemSetNFAMapping
    {
        // Mapping from Items to NFA state
        std::unordered_map<std::shared_ptr<Item>, std::string> item_set_to_nfa_state;
        // Mapping from NFA state to Items
        std::unordered_map<std::string, std::shared_ptr<Item>> nfa_state_to_item_set;
        // Mapping from ItemSet symbols to NFA character set, 1-to-1 mapping
        std::unordered_map<cfg_model::symbol, std::string> item_set_symbol_to_nfa_character;
        // Mapping from NFA character set to ItemSet symbols, 1-to-1 mapping
        std::unordered_map<std::string, cfg_model::symbol> nfa_character_to_item_set_symbol;
    };

    struct ItemSetDFAMapping
    {
        // Mapping from Items to DFA state, these are one-to-many mappings
        std::unordered_map<std::shared_ptr<Item>, std::unordered_set<std::string>> item_set_to_dfa_state;
        // Mapping from DFA state to Items, these are one-to-many mappings
        std::unordered_map<std::string, std::unordered_set<std::shared_ptr<Item>>> dfa_state_to_item_set;
        // Mapping from ItemSet symbols to DFA character set, 1-to-1 mapping
        std::unordered_map<cfg_model::symbol, std::string> item_set_symbol_to_dfa_character;
        // Mapping from DFA character set to ItemSet symbols, 1-to-1 mapping
        std::unordered_map<std::string, cfg_model::symbol> dfa_character_to_item_set_symbol;
    };


    struct ItemSetNFAGenerationResult
    {
        nfa_model::NFA nfa;
        ItemSetNFAMapping item_set_nfa_mapping;
    };

    struct ItemSetDFAGenerationResult
    {
        dfa_model::DFA<std::string> dfa;
        ItemSetDFAMapping item_set_dfa_mapping;
    };

    struct Action
    {
        std::string action_type = "";
        std::string target_state = "";                       // if the action type is "shift"
        cfg_model::symbol reduce_rule_lhs;                   // if the action type is "reduce"
        std::vector<cfg_model::symbol> reduce_rule_rhs = {}; // if the action type is "reduce"

        // ==
        bool operator==(const Action &other) const
        {
            return action_type == other.action_type &&
                   target_state == other.target_state &&
                   reduce_rule_lhs == other.reduce_rule_lhs &&
                   reduce_rule_rhs == other.reduce_rule_rhs;
        }
    };
}

// hash function for lr_parsing_model::Action
namespace std
{
    template <>
    struct hash<lr_parsing_model::Action>
    {
        size_t operator()(const lr_parsing_model::Action &action) const
        {
            size_t seed = 0;
            seed ^= hash<std::string>()(action.action_type) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash<std::string>()(action.target_state) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash<cfg_model::symbol>()(action.reduce_rule_lhs) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            for (const auto &s : action.reduce_rule_rhs)
            {
                seed ^= hash<cfg_model::symbol>()(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

namespace lr_parsing_model
{
    struct LRParsingTable
    {
        std::unordered_set<cfg_model::symbol> all_symbols;
        std::unordered_set<std::string> all_states;                                                                         // all states in the parsing table
        std::unordered_map<std::string, std::unordered_map<cfg_model::symbol, std::unordered_set<Action>>> action_table;    // conflict tolerant action table
        std::unordered_map<std::string, std::unordered_map<cfg_model::symbol, std::unordered_set<std::string>>> goto_table; // goto table, conflict tolerant
        std::string start_state; // the start state of the parsing table

        bool add_action(const std::string &state, const cfg_model::symbol &symbol, const Action &action);

        bool add_goto(const std::string &state, const cfg_model::symbol &symbol, const std::string &next_state);

        bool check_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const;

        bool check_action_table_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const;

        bool check_goto_table_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const;

        std::unordered_set<Action> get_actions(const std::string &state, const cfg_model::symbol &symbol) const;

        std::unordered_set<std::string> get_gotos(const std::string &state, const cfg_model::symbol &symbol) const;

        bool filling_check() const;

        // find all the conflicts in the action table
        std::multimap<std::string, cfg_model::symbol> find_conflicts() const;
    };

    // map each parsing table cell to a set of items
    struct ItemSetParsingTableMapping
    {
        // Mapping from parsing table cell to ItemSet
        std::unordered_map<std::string, std::unordered_map<cfg_model::symbol, std::unordered_set<std::shared_ptr<Item>>>> parsing_table_cell_to_item_set;
        // Mapping from ItemSet to parsing table cell
        std::unordered_map<std::shared_ptr<Item>, std::multimap<std::string, cfg_model::symbol>> item_set_to_parsing_table_cell;
    };

    struct LRParsingTableGenerationResult
    {
        LRParsingTable parsing_table;
        ItemSetParsingTableMapping item_set_parsing_table_mapping;
    };
};

#endif // !LR_PARSING_MODEL_H