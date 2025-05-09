#include "lr_parsing_model.h"
#include "cfg_model.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <memory>

class ItemSetGenerator
{
public:
    // Constructor
    ItemSetGenerator();
    ~ItemSetGenerator();

    lr_parsing_model::ItemSet generate_item_set(const cfg_model::CFG &cfg);
};

// helper functions
namespace itemset_generator_helper
{
    // expand the cfg, add a new initial symbol and its production rules
    cfg_model::CFG expand_cfg(const cfg_model::CFG &cfg);

    // generate the item set for the given cfg
    std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> generate_cfg_items(const cfg_model::CFG &cfg);

    // generate the item set for a given production rule(not epsilon)
    std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> gen_non_epsilon_production_rule_items(const cfg_model::symbol &left_side_symbol, const std::vector<cfg_model::symbol> &right_side_sequence);

    // generate the item set for a given production rule(epsilon)
    std::unordered_set<std::shared_ptr<lr_parsing_model::Item>> gen_epsilon_production_rule_items(const cfg_model::symbol &left_side_symbol);
}