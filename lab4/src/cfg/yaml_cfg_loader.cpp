#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "yaml_cfg_loader.h"
#include <fstream>

// constructor & destructor, all default
YAML_CFG_Loader::YAML_CFG_Loader() = default;
YAML_CFG_Loader::~YAML_CFG_Loader() = default;

// load the CFG from a YAML file
cfg_model::CFG YAML_CFG_Loader::LoadCFG(const std::string &filename)
{
    try
    {
        // check the validity of the YAML file
        YAML_CFG_Loader_Helper::CheckYAMLFile(filename);
        // parse the YAML file and load the CFG
        cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
        // check the validity of the CFG
        YAML_CFG_Loader_Helper::CheckCFG(cfg);
        return cfg;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error loading CFG from file: " + filename + "\n" + e.what();
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

// check the validity of the YAML file
void YAML_CFG_Loader_Helper::CheckYAMLFile(const std::string &filename)
{
    // check if the file exists
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("File not found: " + filename);
    }

    YAML::Node config;
    try
    {
        config = YAML::LoadFile(filename);
    }
    catch (const YAML::ParserException &e)
    {
        throw std::runtime_error("YAML parsing error: " + std::string(e.what()));
    }
    // check for cfg key
    if (!config["cfg"])
    {
        throw std::runtime_error("YAML file does not contain 'cfg' key");
    }
    // in cfg key check for initial_symbol, terminals, non_terminals, and production_rules
    if (!config["cfg"]["initial_symbol"] ||
        !config["cfg"]["terminals"] ||
        !config["cfg"]["non_terminals"] ||
        !config["cfg"]["production_rules"])
    {
        throw std::runtime_error("YAML file does not contain 'initial_symbol', 'terminals', 'non_terminals', or 'production_rules' keys");
    }

    // for each production rule check if there is a lhs and rhs
    for (const auto &rule : config["cfg"]["production_rules"])
    {
        if (!rule["lhs"] || !rule["rhs"])
        {
            throw std::runtime_error("YAML file does not contain 'lhs' or 'rhs' keys in production rules");
        }
    }

    spdlog::info("YAML file is valid");
}

// parse the YAML file and load the CFG
cfg_model::CFG YAML_CFG_Loader_Helper::ParseYAMLFile(const std::string &filename)
{
    try
    {
        cfg_model::CFG temp_cfg;
        YAML::Node config = YAML::LoadFile(filename);
        std::unordered_set<std::string> terminal_names;
        std::unordered_set<std::string> non_terminal_names;

        // load the initial symbol
        temp_cfg.start_symbol.name = config["cfg"]["initial_symbol"].as<std::string>();
        temp_cfg.start_symbol.is_terminal = false;
        spdlog::debug("Initial symbol loaded: {}", temp_cfg.start_symbol.name);
        // load the terminals
        for (const auto &terminal : config["cfg"]["terminals"])
        {
            cfg_model::symbol temp_symbol;
            temp_symbol.name = terminal.as<std::string>();
            temp_symbol.is_terminal = true;
            temp_cfg.terminals.insert(temp_symbol);
            terminal_names.insert(temp_symbol.name);
        }
        std:: string terminal_names_str;
        for (const auto &terminal : temp_cfg.terminals)
        {
            terminal_names_str += terminal.name + " ";
        }
        spdlog::debug("Terminals loaded: {}", terminal_names_str);
        // load the non-terminals
        for (const auto &non_terminal : config["cfg"]["non_terminals"])
        {
            cfg_model::symbol temp_symbol;
            temp_symbol.name = non_terminal.as<std::string>();
            temp_symbol.is_terminal = false;
            temp_cfg.non_terminals.insert(temp_symbol);
            non_terminal_names.insert(temp_symbol.name);
        }
        std::string non_terminal_names_str;
        for (const auto &non_terminal : temp_cfg.non_terminals)
        {
            non_terminal_names_str += non_terminal.name + " ";
        }
        spdlog::debug("Non-terminals loaded: {}", non_terminal_names_str);
        // load the production rules
        for (const auto &rule : config["cfg"]["production_rules"])
        {
            // lhs
            cfg_model::symbol lhs;
            lhs.name = rule["lhs"].as<std::string>();
            lhs.is_terminal = false;

            // parse the rhs
            std::vector<cfg_model::symbol> rhs_symbols;
            bool is_epsilon_production = false;
            // iterate over the rhs sequence
            for (const auto &rhs : rule["rhs"])
            {
                // if the current rhs is ""(empty string), check if it is the only rhs
                // if so, make the lhs epsilon production symbol, otherwise throw an error
                if (rhs.as<std::string>() == "")
                {
                    if (rule["rhs"].size() == 1)
                    {
                        is_epsilon_production = true;
                        spdlog::debug("Epsilon production symbol found at lhs: {}", lhs.name);
                    }
                    else if (is_epsilon_production)
                    {
                        throw std::runtime_error("YAML file contains multiple rhs for epsilon production symbol at lhs: " + lhs.name);
                    }
                    else
                    {
                        throw std::runtime_error("YAML file contains multiple rhs for epsilon production symbol at lhs: " + lhs.name);
                    }
                }
                else
                {
                    // load the rhs symbol into the vector
                    cfg_model::symbol temp_symbol;
                    temp_symbol.name = rhs.as<std::string>();
                    // check if the symbol is terminal or non-terminal
                    if (terminal_names.find(temp_symbol.name) != terminal_names.end())
                    {
                        temp_symbol.is_terminal = true;
                    }
                    else if (non_terminal_names.find(temp_symbol.name) != non_terminal_names.end())
                    {
                        temp_symbol.is_terminal = false;
                    }
                    else
                    {
                        throw std::runtime_error("YAML file contains invalid symbol in rhs: " + temp_symbol.name);
                    }
                    rhs_symbols.push_back(temp_symbol);
                }
            }

            // finalize the production rule
            if (is_epsilon_production)
            {
                temp_cfg.epsilon_production_symbols.insert(lhs);
                spdlog::debug("Epsilon production symbol loaded: {}", lhs.name);
            }
            else
            {
                temp_cfg.production_rules[lhs].insert(rhs_symbols);
                std::string rhs_str;
                for (const auto &symbol : rhs_symbols)
                {
                    rhs_str += symbol.name + " ";
                }
                spdlog::debug("Production rule loaded: {} -> {}", lhs.name, rhs_str);
            }
        }

        spdlog::debug("Production rules loaded: {}", temp_cfg.production_rules.size());
        return temp_cfg;
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Error parsing YAML file: " + filename + "\n" + e.what();
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
}

void YAML_CFG_Loader_Helper::CheckCFG(const cfg_model::CFG &cfg)
{
    try
    {
        // 1. check the mutual exclusivity of terminals and non-terminals
        std::unordered_set<std::string> terminal_names;
        std::unordered_set<std::string> non_terminal_names;
        for (const auto &terminal : cfg.terminals)
        {
            terminal_names.insert(terminal.name);
        }
        for (const auto &non_terminal : cfg.non_terminals)
        {
            non_terminal_names.insert(non_terminal.name);
        }
        for (const auto &terminal : cfg.terminals)
        {
            if (non_terminal_names.find(terminal.name) != non_terminal_names.end())
            {
                throw std::runtime_error("Terminal and non-terminal have the same name: " + terminal.name);
            }
        }
        for (const auto &non_terminal : cfg.non_terminals)
        {
            if (terminal_names.find(non_terminal.name) != terminal_names.end())
            {
                throw std::runtime_error("Terminal and non-terminal have the same name: " + non_terminal.name);
            }
        }
        // 2. check the initial symbol is in the non-terminals and not in the terminals
        if (cfg.non_terminals.find(cfg.start_symbol) == cfg.non_terminals.end())
        {
            throw std::runtime_error("Initial symbol is not in the non-terminals: " + cfg.start_symbol.name);
        }
        if (cfg.terminals.find(cfg.start_symbol) != cfg.terminals.end())
        {
            throw std::runtime_error("Initial symbol is in the terminals: " + cfg.start_symbol.name);
        }
        // 3. check the production rules are valid
        for (const auto &rule : cfg.production_rules)
        {
            // check if the lhs is in the non-terminals
            if (non_terminal_names.find(rule.first.name) == non_terminal_names.end())
            {
                throw std::runtime_error("Production rule lhs is not in the non-terminals: " + rule.first.name);
            }
            // check if the rhs has at least one symbol
            if (rule.second.empty())
            {
                throw std::runtime_error("Production rule rhs is empty for lhs: " + rule.first.name);
            }
            // check if the rhs symbols are in the terminals or non-terminals
            for (const auto &rhs : rule.second)
            {
                for (const auto &symbol : rhs)
                {
                    if (terminal_names.find(symbol.name) == terminal_names.end() &&
                        non_terminal_names.find(symbol.name) == non_terminal_names.end())
                    {
                        throw std::runtime_error("Production rule of lhs " + rule.first.name + " has invalid symbol in rhs: " + symbol.name);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "CFG validation error: " + std::string(e.what());
        spdlog::error(error_msg);
        throw std::runtime_error(error_msg);
    }
    spdlog::info("CFG final checking passed");
}
