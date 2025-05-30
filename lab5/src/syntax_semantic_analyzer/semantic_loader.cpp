#include "semantic_loader.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"
#include <stdexcept>

syntax_semantic_model::ProductionInfoMapping load_semantic_info(const std::string& filename, cfg_model::CFG& target_cfg){
    try
    {
        syntax_semantic_model::ProductionInfoMapping production_info_mapping;
        YAML::Node config = YAML::LoadFile(filename);
        int total_loaded_production_rules = 0;

        // load the production rules
        for (const auto &rule : config["cfg"]["production_rules"])
        {
            // lhs
            cfg_model::symbol lhs;
            lhs.name = rule["lhs"].as<std::string>();
            lhs.is_terminal = false;
            std::string semantic_info;
            semantic_info = rule["node_type"].as<std::string>();
            // empty semantic info is not allowed
            if (semantic_info.empty())
            {
                std::string error_msg = "Semantic info for production rule is empty: " + lhs.name;
                spdlog::error(error_msg);
                throw std::runtime_error(error_msg);
            }

            // parse the rhs
            std::vector<cfg_model::symbol> rhs_symbols;
            bool is_epsilon_production = false;
            // iterate over the rhs sequence
            for (const auto &rhs : rule["rhs"])
            {
                // if the current rhs is ""(empty string)
                if (rhs.as<std::string>() == "")
                {
                    is_epsilon_production = true;
                }
                else
                {
                    // load the rhs symbol into the vector
                    cfg_model::symbol temp_symbol;
                    temp_symbol.name = rhs.as<std::string>();
                    temp_symbol.is_terminal = false; // default is non-terminal
                    temp_symbol.special_property = ""; // default is empty
                    // find the symbol in the target_cfg
                    auto it = target_cfg.terminals.find(temp_symbol);
                    if (it != target_cfg.terminals.end())
                    {
                        temp_symbol.is_terminal = true;
                        temp_symbol.special_property = it->special_property; // copy the special property if exists
                        spdlog::debug("Terminal symbol loaded: {}", temp_symbol.name);
                    }
                    else
                    {
                        it = target_cfg.non_terminals.find(temp_symbol);
                        if (it == target_cfg.non_terminals.end())
                        {
                            // if the symbol is not found in either terminals or non_terminals, throw an error
                            std::string error_msg = "Symbol not found in CFG: " + temp_symbol.name;
                            spdlog::error(error_msg);
                            throw std::runtime_error(error_msg);
                        }
                        else
                        {
                            temp_symbol.is_terminal = false; // it's a non-terminal
                            spdlog::debug("Non-terminal symbol loaded: {}", temp_symbol.name);
                            temp_symbol.special_property = it->special_property; // copy the special property if exists
                        }
                    }
                    rhs_symbols.push_back(temp_symbol);
                }
            }

            // finalize the production rule
            if (is_epsilon_production)
            {
                production_info_mapping.production_info[lhs][{}] = semantic_info;
                spdlog::debug("Epsilon production rule loaded: {} -> ε with node type : {}", lhs.name, semantic_info);
                total_loaded_production_rules++;
            }
            else
            {
                production_info_mapping.production_info[lhs][rhs_symbols] = semantic_info;
                std::string rhs_str;
                for (const auto &symbol : rhs_symbols)
                {
                    rhs_str += symbol.name + " ";
                }
                spdlog::debug("Production rule loaded: {} -> {} with node type : {}", lhs.name, rhs_str, semantic_info);
                total_loaded_production_rules++;
            }
        }

        spdlog::debug("Production rules loaded: {}", total_loaded_production_rules);
        return production_info_mapping;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error parsing YAML file: " + filename + "\n" + e.what();
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}