#ifndef SIMPLE_LR_PARSING_TABLE_GENERATOR_H
#define SIMPLE_LR_PARSING_TABLE_GENERATOR_H

#include "lr_parsing_model.h"
#include "lr_parsing_table_generator.h"
#include "itemset_generator.h"
#include "itemset_to_parsing_table.h"
#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"

class SimpleLRParsingTableGenerator : public LRParsingTableGenerator
{
public:
    SimpleLRParsingTableGenerator() = default;
    ~SimpleLRParsingTableGenerator() override = default;

    lr_parsing_model::LRParsingTable generate_parsing_table(const cfg_model::CFG &cfg) override;
    lr_parsing_model::ItemSet generate_item_set(const cfg_model::CFG &cfg) override;

    // using the NFA-DFA method, it can also be used to generate corresponding NFA/DFA
    lr_parsing_model::ItemSetNFAGenerationResult generate_item_set_nfa(const cfg_model::CFG &cfg);
    lr_parsing_model::ItemSetDFAGenerationResult generate_item_set_dfa(const cfg_model::CFG &cfg);
};
#endif // !SIMPLE_LR_PARSING_TABLE_GENERATOR_H