#include "cfg_analyzer.h"
#include "cfg_model.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

CFGAnalyzer::CFGAnalyzer(const cfg_model::CFG &cfg) : cfg(cfg)
{
    // Initialize the first and follow sets
    first_set = cfg_model::FirstSet();
    follow_set = cfg_model::FollowSet();
}
CFGAnalyzer::~CFGAnalyzer() = default;

// Get the first set
const cfg_model::FirstSet &CFGAnalyzer::getFirstSet() const
{
    return first_set;
}
// Get the follow set
const cfg_model::FollowSet &CFGAnalyzer::getFollowSet() const
{
    return follow_set;
}

// Compute the first set for the CFG
void CFGAnalyzer::computeFirstSet()
{
    try{
        spdlog::debug("Computing first set...");
        // clear the first set
        first_set.first_set.clear();
        first_set.symbols_with_epsilon.clear();
        // Initialize the first set: for each non-terminal, the first set is empty, for each terminal, the first set is the terminal itself
        for (const auto &non_terminal : cfg.non_terminals)
        {
            first_set.first_set[non_terminal] = std::unordered_set<cfg_model::symbol>();
            // if the non-terminal has epsilon production, add it to the epsilon set
            if (cfg.epsilon_production_symbols.find(non_terminal) != cfg.epsilon_production_symbols.end())
            {
                first_set.symbols_with_epsilon.insert(non_terminal);
            }
        }
        for (const auto &terminal : cfg.terminals)
        {
            first_set.first_set[terminal] = std::unordered_set<cfg_model::symbol>({terminal});
        }

        // start iterating over the production rules until no changes are made
        bool changed = true;
        while(changed)
        {
            changed = false;
            // iterate over the non-espilon production rules
            for (const auto &rule : cfg.production_rules)
            {
                const cfg_model::symbol &lhs = rule.first;
                // iterate over all the candidate productions
                for (const auto &rhs : rule.second)
                {
                    const cfg_model::symbol &first_symbol = rhs[0];
                    // add every symbol in the first set of the first symbol to the first set of the lhs
                    if (first_set.first_set.at(first_symbol).size() == 0)
                    {
                        // the first set is still empty
                        continue;
                    }
                    int current_lhs_first_set_size = first_set.first_set[lhs].size();
                    int current_epsilon_set_size = first_set.symbols_with_epsilon.size();
                    // add the first set of the first symbol to the first set of the lhs
                    first_set.first_set[lhs].insert(first_set.first_set[first_symbol].begin(), first_set.first_set[first_symbol].end());

                    // also, check if the first symbol set has epsilon
                    if (first_set.symbols_with_epsilon.find(first_symbol) != first_set.symbols_with_epsilon.end())
                    {
                        // 1. now an epsilon symbol must in the first set of the first symbol, so add it to the first set of the lhs
                        first_set.symbols_with_epsilon.insert(lhs);
                        // 2. check if there is a second symbol in the rhs
                        if (rhs.size() > 1)
                        {
                            // 3. if there is a second symbol, add the first set of the second symbol to the first set of the lhs
                            const cfg_model::symbol &second_symbol = rhs[1];
                            // add the first set of the second symbol to the first set of the lhs
                            first_set.first_set[lhs].insert(first_set.first_set[second_symbol].begin(), first_set.first_set[second_symbol].end());

                            // 4. !!! important: check if the second symbol has epsilon
                            if (first_set.symbols_with_epsilon.find(second_symbol) != first_set.symbols_with_epsilon.end())
                            {
                                // 5. if the second symbol has epsilon, add it to the first set of the lhs
                                first_set.symbols_with_epsilon.insert(lhs);
                            }
                        }
                    }

                    // check if the first set of the lhs has changed
                    if (first_set.first_set[lhs].size() != current_lhs_first_set_size || first_set.symbols_with_epsilon.size() != current_epsilon_set_size)
                    {
                        changed = true;
                    }

                }
                
            }
        }

        spdlog::debug("First set computed successfully.");
        // log the first set
        for (const auto &entry : first_set.first_set)
        {
            spdlog::debug("First set of {}: ", std::string(entry.first));
            for (const auto &symbol : entry.second)
            {
                spdlog::debug("{}", std::string(symbol));
            }
            if (first_set.symbols_with_epsilon.find(entry.first) != first_set.symbols_with_epsilon.end())
            {
                spdlog::debug("Epsilon production found for {}", std::string(entry.first));
            }
        }

        return;

    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error computing first set: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

// Compute the follow set for the CFG
void CFGAnalyzer::computeFollowSet()
{
    try{
        spdlog::debug("Computing follow set...");
        // check if the first set is computed
        if (first_set.first_set.size() == 0)
        {
            throw std::runtime_error("First set is not computed. Please compute the first set before computing the follow set.");
        }
        // clear the follow set
        follow_set.follow_set.clear();
        // find the end symbol
        cfg_model::symbol end_symbol;
        bool end_symbol_found = false;
        for (const auto &terminal : cfg.terminals)
        {
            if (terminal.special_property == "END")
            {
                end_symbol = terminal;
                end_symbol_found = true;
                break;
            }
        }
        if (!end_symbol_found)
        {
            throw std::runtime_error("End symbol not found in the CFG. Please add an end symbol to the CFG.");
        }
        // initialize the follow set for each non-terminal
        for (const auto &non_terminal : cfg.non_terminals)
        {
            follow_set.follow_set[non_terminal] = std::unordered_set<cfg_model::symbol>();
        }
        // add the end symbol to the follow set of the start symbol
        follow_set.follow_set[cfg.start_symbol].insert(end_symbol);
        follow_set.end_symbol = end_symbol;
        // start iterating over the production rules until no changes are made
        bool changed = true;
        while (changed)
        {
            changed = false;
            // iterate over the production rules
            for (const auto &rule : cfg.production_rules)
            {
                const cfg_model::symbol &lhs = rule.first;
                // iterate over all the candidate productions
                for (const auto &rhs : rule.second)
                {
                    std::string rhs_str = "";
                    for (const auto &symbol : rhs)
                    {
                        rhs_str += std::string(symbol) + " ";
                    }
                    spdlog::debug("Processing production rule: {} -> {}", std::string(lhs), rhs_str);
                    // iterate over the symbols in the rhs
                    for (size_t i = 0; i < rhs.size(); i++)
                    {
                        const cfg_model::symbol &current_symbol = rhs[i];
                        // if is terminal, continue
                        if (current_symbol.is_terminal)
                        {
                            continue;
                        }
                        bool epsilon_suffix = false;
                        bool is_last_symbol = false;
                        // if the current symbol is the last symbol, then it has epsilon suffix
                        if (i == rhs.size() - 1)
                        {
                            is_last_symbol = true;
                            epsilon_suffix = true;
                        }
                        // else if the next symbol is in the symbol with epsilon set, then it has epsilon suffix
                        else if (first_set.symbols_with_epsilon.find(rhs[i + 1]) != first_set.symbols_with_epsilon.end())
                        {
                            spdlog::debug("Symbol {} has epsilon suffix", std::string(current_symbol));
                            epsilon_suffix = true;
                        }
                        // else it does not have epsilon suffix
                        else
                        {
                            epsilon_suffix = false;
                        }

                        // if the current symbol has an epsilon suffix, then add the follow set of the lhs to the follow set of the current symbol
                        if (epsilon_suffix)
                        {
                            int current_follow_set_size = follow_set.follow_set[current_symbol].size();
                            // add the follow set of the lhs to the follow set of the current symbol
                            follow_set.follow_set[current_symbol].insert(follow_set.follow_set[lhs].begin(), follow_set.follow_set[lhs].end());
                            // if not the last symbol, add the first set of the next symbol to the follow set of the current symbol
                            if (!is_last_symbol)
                            {
                                const cfg_model::symbol &next_symbol = rhs[i + 1];
                                // add the first set of the next symbol to the follow set of the current symbol
                                follow_set.follow_set[current_symbol].insert(first_set.first_set[next_symbol].begin(), first_set.first_set[next_symbol].end());
                            }
                            // check if the follow set of the current symbol has changed
                            if (follow_set.follow_set[current_symbol].size() != current_follow_set_size)
                            {
                                changed = true;
                            }
                        }
                        // if the current symbol does not have an epsilon suffix, then add the first set of the next symbol to the follow set of the current symbol
                        else
                        {
                            // the symbol must not be the last symbol
                            const cfg_model::symbol &next_symbol = rhs[i + 1];
                            // add the first set of the next symbol to the follow set of the current symbol
                            int current_follow_set_size = follow_set.follow_set[current_symbol].size();
                            // here our first set does not contain epsilon, so we can add it directly
                            spdlog::debug("Adding first set of {} to follow set of {}", std::string(next_symbol), std::string(current_symbol));
                            follow_set.follow_set[current_symbol].insert(first_set.first_set[next_symbol].begin(), first_set.first_set[next_symbol].end());
                            // check if the follow set of the current symbol has changed
                            if (follow_set.follow_set[current_symbol].size() != current_follow_set_size)
                            {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        spdlog::debug("Follow set computed successfully.");
        // log the follow set
        for (const auto &entry : follow_set.follow_set)
        {
            spdlog::debug("Follow set of {}: ", std::string(entry.first));
            for (const auto &symbol : entry.second)
            {
                spdlog::debug("{}", std::string(symbol));
            }
        }
        return;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error computing follow set: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}