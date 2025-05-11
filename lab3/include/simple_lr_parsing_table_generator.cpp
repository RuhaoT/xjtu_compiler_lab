#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "itemset_generator.h"
#include "itemset_to_parsing_table.h"
#include "simple_lr_parsing_table_generator.h"
#include "spdlog/spdlog.h"

lr_parsing_model::ItemSet SimpleLRParsingTableGenerator::generate_item_set(const cfg_model::CFG &cfg)
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

lr_parsing_model::LRParsingTable SimpleLRParsingTableGenerator::generate_parsing_table(const cfg_model::CFG &cfg)
{
    try
    {
        // Generate the item set
        lr_parsing_model::ItemSet item_set = generate_item_set(cfg);

        // Create an instance of ItemSetToParsingTable
        ItemSetToParsingTable item_set_to_parsing_table(item_set);

        // Generate the parsing table
        lr_parsing_model::LRParsingTable parsing_table = item_set_to_parsing_table.get_parsing_table();

        return parsing_table;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating parsing table: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::ItemSetNFAGenerationResult SimpleLRParsingTableGenerator::generate_item_set_nfa(const cfg_model::CFG &cfg)
{
    try
    {
        // Generate the item set
        lr_parsing_model::ItemSet item_set = generate_item_set(cfg);

        // Create an instance of ItemSetToParsingTable
        ItemSetToParsingTable item_set_to_parsing_table(item_set);

        // Generate the NFA
        lr_parsing_model::ItemSetNFAGenerationResult nfa_result = item_set_to_parsing_table.get_nfa();

        return nfa_result;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating item set NFA: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

lr_parsing_model::ItemSetDFAGenerationResult SimpleLRParsingTableGenerator::generate_item_set_dfa(const cfg_model::CFG &cfg)
{
    try
    {
        // Generate the item set
        lr_parsing_model::ItemSet item_set = generate_item_set(cfg);

        // Create an instance of ItemSetToParsingTable
        ItemSetToParsingTable item_set_to_parsing_table(item_set);

        // Generate the DFA
        lr_parsing_model::ItemSetDFAGenerationResult dfa_result = item_set_to_parsing_table.get_dfa();

        return dfa_result;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating item set DFA: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}