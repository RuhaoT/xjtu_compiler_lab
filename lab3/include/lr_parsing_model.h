#ifndef LR_PARSING_MODEL_H
#define LR_PARSING_MODEL_H

#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
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
        operator std::string() const
        {
            std::string result = "[" + left_side_symbol.name + " -> ";
            for (const auto &s : sequence_already_parsed)
            {
                result += std::string(s) + " ";
            }
            result += " · ";
            for (const auto &s : sequence_to_parse)
            {
                result += " " + std::string(s);
            }
            result += "]";
            return result;
        }
    };

} // namespace lr_parsing_model

// Specialization of std::hash for lr_parsing_model::Item
namespace std
{
    template <>
    struct hash<lr_parsing_model::Item>
    {
        size_t operator()(const lr_parsing_model::Item &item) const
        {
            size_t seed = item.left_side_symbol.name.size();
            for (const auto &s : item.sequence_already_parsed)
            {
                seed ^= hash<cfg_model::symbol>()(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            for (const auto &s : item.sequence_to_parse)
            {
                seed ^= hash<cfg_model::symbol>()(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
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
        // Mapping from Items to DFA state, these are many-to-one mappings
        std::unordered_map<std::shared_ptr<Item>, std::string> item_set_to_dfa_state;
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
        dfa_model::ConflictTolerantDFA<std::string> dfa;
        ItemSetDFAMapping item_set_dfa_mapping;
    };
}

#endif // !LR_PARSING_MODEL_H