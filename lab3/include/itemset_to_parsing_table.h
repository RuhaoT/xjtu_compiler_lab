#ifndef ITEMSET_TO_PARSING_TABLE_H
#define ITEMSET_TO_PARSING_TABLE_H

#include "lr_parsing_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "standard_nfa_ctdfa_converter.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// this class not only generates the parsing table, but also provides the corresponding NFA/DFA for analysis and generation process visualization
// one instance of this class is dedicated to one item set
class ItemSetToParsingTable
{
private:
    lr_parsing_model::ItemSet item_set;
    nfa_model::NFA nfa;
    lr_parsing_model::ItemSetNFAMapping item_set_nfa_mapping;
    dfa_model::ConflictTolerantDFA<std::string> dfa;
    lr_parsing_model::ItemSetDFAMapping item_set_dfa_mapping;
    lr_parsing_model::LRParsingTable parsing_table;

public:
    ItemSetToParsingTable(const lr_parsing_model::ItemSet &item_set);
    ~ItemSetToParsingTable() = default;

    // generate corresponding NFA for the item set
    lr_parsing_model::ItemSetNFAGenerationResult generate_nfa();

    // generate corresponding DFA for the item set
    lr_parsing_model::ItemSetDFAGenerationResult generate_dfa();



    lr_parsing_model::LRParsingTable build_parsing_table();

        // get nfa & mapping
    lr_parsing_model::ItemSetNFAGenerationResult get_nfa() const
    {
        return {nfa, item_set_nfa_mapping};
    }
    // get dfa & mapping
    lr_parsing_model::ItemSetDFAGenerationResult get_dfa() const
    {
        return {dfa, item_set_dfa_mapping};
    }
    // get parsing table
    lr_parsing_model::LRParsingTable get_parsing_table() const
    {
        return parsing_table;
    }
};

namespace itemset_to_parsing_table_helper
{
    // help generate a unique NFA character name for a given item
    std::string generate_nfa_character_name(const cfg_model::symbol &symbol);
    // help generate a unique NFA state name for a given item
    std::string generate_nfa_state_name(const lr_parsing_model::Item &item);
}

#endif // !ITEMSET_TO_PARSING_TABLE_H