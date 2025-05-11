#ifndef LR_PARSING_TABLE_GENERATOR_H
#define LR_PARSING_TABLE_GENERATOR_H

#include "lr_parsing_model.h"
#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

class LRParsingTableGenerator
{
public:
    virtual ~LRParsingTableGenerator() = default;

    virtual lr_parsing_model::LRParsingTable generate_parsing_table(const cfg_model::CFG &cfg) = 0;

    virtual lr_parsing_model::ItemSet generate_item_set(const cfg_model::CFG &cfg) = 0;
};

#endif // !LR_PARSING_TABLE_GENERATOR_H