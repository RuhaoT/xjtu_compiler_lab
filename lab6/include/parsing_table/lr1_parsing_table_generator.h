#ifndef LR1_PARSING_TABLE_GENERATOR_H
#define LR1_PARSING_TABLE_GENERATOR_H

#include "lr_parsing_model.h"
#include "lr_parsing_table_generator.h"
#include "itemset_generator.h"
#include "itemset_to_parsing_table.h"
#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "standard_nfa_dfa_converter.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

class LR1ItemPool {
public:
    // 通过内容获取唯一shared_ptr
    std::shared_ptr<lr_parsing_model::LR1Item> get_or_create(const lr_parsing_model::Item& item, const cfg_model::symbol& lookahead) {
        lr_parsing_model::LR1Item key(item, lookahead);
        return get_or_create_impl(key);
    }
    std::shared_ptr<lr_parsing_model::LR1Item> get_or_create(const lr_parsing_model::Item& item, const std::unordered_set<cfg_model::symbol>& lookahead) {
        lr_parsing_model::LR1Item key(item, lookahead);
        return get_or_create_impl(key);
    }
    std::shared_ptr<lr_parsing_model::LR1Item> get_or_create(const lr_parsing_model::LR1Item& item) {
        return get_or_create_impl(item);
    }
    std::shared_ptr<lr_parsing_model::LR1Item> get_or_create(const std::shared_ptr<lr_parsing_model::LR1Item>& item_ptr) {
        return get_or_create_impl(*item_ptr);
    }
private:
    std::unordered_map<lr_parsing_model::LR1Item, std::shared_ptr<lr_parsing_model::LR1Item>> pool_;
    std::mutex mutex_;
    std::shared_ptr<lr_parsing_model::LR1Item> get_or_create_impl(const lr_parsing_model::LR1Item& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = pool_.find(key);
        if (it != pool_.end()) return it->second;
        auto ptr = std::make_shared<lr_parsing_model::LR1Item>(key);
        pool_.emplace(*ptr, ptr);
        return ptr;
    }
};

class LR1ParsingTableGenerator : public LRParsingTableGenerator
{
public:
    LR1ParsingTableGenerator() = default;
    ~LR1ParsingTableGenerator() override = default;

    lr_parsing_model::LRParsingTable generate_parsing_table(const cfg_model::CFG &cfg) override;
    lr_parsing_model::ItemSet generate_item_set(const cfg_model::CFG &cfg) override;

    // the LR1 table generation is completely DFA-based, thus not providing NFA generation
    lr_parsing_model::LR1ItemSetDFAGenerationResult generate_item_set_dfa(const cfg_model::CFG &cfg);

private:
    LR1ItemPool pool_; // 新增：LR1Item对象池
};

namespace lr1_parsing_table_generator_helper
{
    // the strategy for dealing with conflicts that cannot be resolved by the LR(1) parsing table generator
    enum class LR1ConflictResolutionStrategy
    {
        SHIFT_OVER_REDUCE, // shift over reduce
        REDUCE_OVER_SHIFT, // reduce over shift
    };
    // generate LR(1) parsing table to item set mapping
    lr_parsing_model::ItemSetParsingTableMapping generate_item_set_parsing_table_mapping(
        const lr_parsing_model::LRParsingTable &parsing_table,
        const lr_parsing_model::ItemSet &item_set,
        const lr_parsing_model::ItemSetDFAMapping &item_set_dfa_mapping
    );
    // resolve conflicts in the LR(1) parsing table
    lr_parsing_model::LRParsingTable resolve_conflicts(
        const lr_parsing_model::LRParsingTable &parsing_table_to_be_resolved,
        const lr_parsing_model::ItemSetParsingTableMapping &item_set_parsing_table_mapping,
        const LR1ConflictResolutionStrategy &conflict_resolution_strategy = LR1ConflictResolutionStrategy::SHIFT_OVER_REDUCE
    );
        
    // helper functions for LR(1) parsing table generation
    // generate a blank LR(1) item set from an LR(0) item set
    lr_parsing_model::ItemSet generate_blank_lr1_item_set(const lr_parsing_model::ItemSet &item_set, LR1ItemPool& pool);

    // generate a set of LR(1) items from an LR(0) item and their lookahead symbols
    std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> grow_closure(
        const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &initial_items,
        const lr_parsing_model::ItemSet &reference_lr0_item_set,
        const cfg_model::CFG &cfg,
        const cfg_model::FirstSet &first_set,
        const cfg_model::FollowSet &follow_set,
        LR1ItemPool& pool);

    // generate the initial closure items from an existing closure by moving in one symbol
    std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> generate_initial_closure(
        const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &closure_items,
        const lr_parsing_model::ItemSet &reference_lr0_item_set,
        const cfg_model::symbol &next_symbol,
        const cfg_model::CFG &cfg,
        const cfg_model::FirstSet &first_set,
        const cfg_model::FollowSet &follow_set,
        LR1ItemPool& pool);

    // generate a unique DFA name string for an LR(1) closure set
    std::string generate_lr1_closure_name(const std::unordered_set<std::shared_ptr<lr_parsing_model::LR1Item>> &closure_items);
}

#endif // !LR1_PARSING_TABLE_GENERATOR_H