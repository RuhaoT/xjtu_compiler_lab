#include "itemset_generator.h"
#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <vector>

// constructor and destructor, all default
ItemSetGenerator::ItemSetGenerator() = default;
ItemSetGenerator::~ItemSetGenerator() = default;

lr_parsing_model::ItemSet ItemSetGenerator::generate_item_set(const cfg_model::CFG &cfg)
{
    // generate the item set for the given cfg
    lr_parsing_model::ItemSet item_set;
    // 1. expand the cfg
    cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);
    cfg_model::symbol original_initial_symbol = cfg.start_symbol;
    cfg_model::symbol new_initial_symbol = expanded_cfg.start_symbol;
    // 2. generate the items for the expanded cfg
    auto items = itemset_generator_helper::generate_cfg_items(expanded_cfg);
    // 3. add the items to the item set
    item_set.items = items;
    spdlog::debug("Item set generated with {} items", items.size());
    // 4. find the items related to the new initial symbol
    std::shared_ptr<lr_parsing_model::Item> parsing_start_item;
    std::shared_ptr<lr_parsing_model::Item> parsing_end_item;
    for (const auto &item : items)
    {
        spdlog::debug("Item: {} -> ", item->left_side_symbol.name);
        if (item->left_side_symbol == new_initial_symbol)
        {
            spdlog::debug("Found the item for the new initial symbol: {} -> ", item->left_side_symbol.name);
            // check if the item is the initial item
            if (item->sequence_already_parsed.empty() && item->sequence_to_parse.size() == 1)
            {
                spdlog::debug("Found the start item: {} -> · {}", item->left_side_symbol.name, item->sequence_to_parse[0].name);
                parsing_start_item = item;
            }
            else if (item->sequence_already_parsed.size() == 1 && item->sequence_already_parsed[0] == original_initial_symbol && item->sequence_to_parse.empty())
            {
                spdlog::debug("Found the end item: {} -> {} ·", item->left_side_symbol.name, item->sequence_already_parsed[0].name);
                parsing_end_item = item;
            }
            else
            {
                std::string error_msg = "Error: The item set is not valid. The new initial symbol has an unexpected item: " + item->left_side_symbol.name + " -> ";
                for (const auto &symbol : item->sequence_already_parsed)
                {
                    error_msg += symbol.name + " ";
                }
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }
        }
    }
    // set the start and end items
    if (parsing_start_item == nullptr || parsing_end_item == nullptr)
    {
        std::string error_msg = "Error: The item set is not valid. The new initial symbol has no start or end item.";
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
    item_set.start_item = parsing_start_item;
    item_set.end_item = parsing_end_item;
    // 5. generate the symbol set
    // all terminals and non-terminals in the item set
    for (const auto &symbol : cfg.non_terminals)
    {
        item_set.symbol_set.insert(symbol);
    }
    for (const auto &symbol : cfg.terminals)
    {
        item_set.symbol_set.insert(symbol);
    }
    return item_set;
}

// helper functions
cfg_model::CFG itemset_generator_helper::expand_cfg(const cfg_model::CFG &cfg)
{
    try
    {
        // note this function creates a new CFG, it does not modify the original CFG
        cfg_model::CFG expanded_cfg = cfg;
        // 1. read the initial symbol
        auto original_initial_symbol = cfg.start_symbol;
        // 2. create a new initial symbol
        std::string new_initial_symbol_name = original_initial_symbol.name + "_expanded";
        // keep adding _expanded to the name until it is unique
        while (expanded_cfg.non_terminals.find({new_initial_symbol_name, false}) != expanded_cfg.non_terminals.end())
        {
            spdlog::debug("New initial symbol name already exists, adding another _expanded to the name");
            new_initial_symbol_name += "_expanded";
        }
        cfg_model::symbol new_initial_symbol;
        new_initial_symbol.name = new_initial_symbol_name;
        new_initial_symbol.is_terminal = false;
        // 3. add the new initial symbol to the CFG
        expanded_cfg.non_terminals.insert(new_initial_symbol);
        expanded_cfg.start_symbol = new_initial_symbol;
        // 4. add a new production rule start_symbol -> original_initial_symbol
        std::vector<cfg_model::symbol> new_production_rule;
        new_production_rule.push_back(original_initial_symbol);
        expanded_cfg.production_rules[new_initial_symbol].insert(new_production_rule);
        // 5. copy the rest of the original CFG
        // copy non-terminals and terminals
        for (const auto &terminal : cfg.terminals)
        {
            expanded_cfg.terminals.insert(terminal);
        }
        for (const auto &non_terminal : cfg.non_terminals)
        {
            expanded_cfg.non_terminals.insert(non_terminal);
        }
        // copy production rules
        for (const auto &rule : cfg.production_rules)
        {
            expanded_cfg.production_rules[rule.first] = rule.second;
        }
        // copy epsilon production symbols
        for (const auto &epsilon_symbol : cfg.epsilon_production_symbols)
        {
            expanded_cfg.epsilon_production_symbols.insert(epsilon_symbol);
        }
        // 6. return the expanded CFG
        return expanded_cfg;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during CFG expansion: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> itemset_generator_helper::gen_epsilon_production_rule_items(const cfg_model::symbol &left_side_symbol)
{
    try
    {
        // generate the item set for a given production rule(epsilon)
        std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> items;
        std::shared_ptr<lr_parsing_model::Item> item = std::make_shared<lr_parsing_model::Item>();
        item->left_side_symbol = left_side_symbol;
        // set two vectors to empty
        item->sequence_already_parsed = {};
        item->sequence_to_parse = {};
        // insert the item into the set
        items.insert(item);
        spdlog::debug("Generated epsilon production rule item: {} -> ε", left_side_symbol.name);
        return items;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during epsilon production rule item generation: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> itemset_generator_helper::gen_non_epsilon_production_rule_items(const cfg_model::symbol &left_side_symbol, const std::vector<cfg_model::symbol> &right_side_sequence)
{
    try
    {
        // generate the item set for a given production rule(not epsilon)
        std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> items;

        // generate all items for the given production rule
        for (size_t i = 0; i <= right_side_sequence.size(); ++i)
        {
            // create a new item
            std::shared_ptr<lr_parsing_model::Item> item = std::make_shared<lr_parsing_model::Item>();
            item->left_side_symbol = left_side_symbol;
            item->sequence_already_parsed = std::vector<cfg_model::symbol>(right_side_sequence.begin(), right_side_sequence.begin() + i);
            item->sequence_to_parse = std::vector<cfg_model::symbol>(right_side_sequence.begin() + i, right_side_sequence.end());
            // insert the item into the set
            items.insert(item);
            std::string parsed_sequence_str = "";
            std::string to_parse_sequence_str = "";
            for (const auto &symbol : item->sequence_already_parsed)
            {
                parsed_sequence_str += symbol.name + " ";
            }
            for (const auto &symbol : item->sequence_to_parse)
            {
                to_parse_sequence_str += symbol.name + " ";
            }
            spdlog::debug("Generated non-epsilon production rule item: {} -> {} · {}", left_side_symbol.name, parsed_sequence_str, to_parse_sequence_str);
        }

        return items;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during non-epsilon production rule item generation: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> itemset_generator_helper::generate_cfg_items(const cfg_model::CFG &cfg)
{
    try
    {
        // generate the item set for the given cfg
        std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> items;

        // 1. generate items for epsilon production rules
        for (const auto &epsilon_symbol : cfg.epsilon_production_symbols)
        {
            auto epsilon_items = gen_epsilon_production_rule_items(epsilon_symbol);
            items.insert(epsilon_items.begin(), epsilon_items.end());
        }
        spdlog::debug("Eplison production rules items generated");

        // 2. generate items for non-epsilon production rules
        for (const auto &rule : cfg.production_rules)
        {
            const auto &left_side_symbol = rule.first;
            const auto &right_side_sequences_set = rule.second;
            for (const auto &right_side_sequence : right_side_sequences_set)
            {
                auto non_epsilon_items = gen_non_epsilon_production_rule_items(left_side_symbol, right_side_sequence);
                items.insert(non_epsilon_items.begin(), non_epsilon_items.end());
            }
        }
        spdlog::debug("Non-eplison production rules items generated");

        return items;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error during CFG item generation: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}