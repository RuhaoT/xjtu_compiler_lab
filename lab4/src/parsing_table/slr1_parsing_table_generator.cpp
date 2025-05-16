#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "itemset_generator.h"
#include "itemset_to_parsing_table.h"
#include "slr1_conflict_resolver.h"
#include "slr1_parsing_table_generator.h"
#include "cfg_analyzer.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

lr_parsing_model::ItemSet SLR1ParsingTableGenerator::generate_item_set(const cfg_model::CFG &cfg)
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

lr_parsing_model::LRParsingTable SLR1ParsingTableGenerator::generate_parsing_table(const cfg_model::CFG &cfg)
{
    try
    {
        // Generate the item set
        lr_parsing_model::ItemSet item_set = generate_item_set(cfg);
        cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);

        // Create an instance of ItemSetToParsingTable
        ItemSetToParsingTable item_set_to_parsing_table(item_set);

        // Generate the parsing table
        lr_parsing_model::LRParsingTableGenerationResult generation_result = item_set_to_parsing_table.get_parsing_table();

        // Resolve conflicts in the parsing table
        SLR1ConflictResolver conflict_resolver(
            generation_result.parsing_table, expanded_cfg, generation_result.item_set_parsing_table_mapping);
        if (!conflict_resolver.resolve_conflicts())
        {
            std::string error_msg = "The parsing table has conflicts and cannot be resolved.";
            spdlog::error(error_msg);
        }
        // Get the resolved parsing table
        auto resolved_parsing_table = conflict_resolver.get_solved_parsing_table();

        return resolved_parsing_table;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error generating parsing table: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}