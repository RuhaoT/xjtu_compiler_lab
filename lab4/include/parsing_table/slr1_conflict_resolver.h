#ifndef SLR1_CONFLICT_SOLVER_H
#define SLR1_CONFLICT_SOLVER_H

#include "cfg_model.h"
#include "cfg_analyzer.h"
#include "lr_parsing_model.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

// one solver is dedicated to one parsing table
class SLR1ConflictResolver
{
private:
    lr_parsing_model::LRParsingTable parsing_table; // the parsing table to be solved
    lr_parsing_model::LRParsingTable resolved_parsing_table; // the parsing table after solving the conflicts
    cfg_model::CFG cfg;                             // the CFG used to generate the parsing table
    lr_parsing_model::ItemSetParsingTableMapping item_set_parsing_table_mapping; // the mapping from item set to parsing table cell

public:
    SLR1ConflictResolver(const lr_parsing_model::LRParsingTable &parsing_table, const cfg_model::CFG &cfg, const lr_parsing_model::ItemSetParsingTableMapping &item_set_parsing_table_mapping);
    ~SLR1ConflictResolver() = default;

    // attempt to solve the conflicts in the parsing table
    bool resolve_conflicts();

    // get the parsing table after solving the conflicts
    lr_parsing_model::LRParsingTable get_solved_parsing_table() const;
};

// helper functions
namespace slr1_conflict_resolver_helper
{
    std::unordered_set<cfg_model::symbol> get_shift_symbols(const std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> &items);

}

#endif // !SLR1_CONFLICT_SOLVER_H