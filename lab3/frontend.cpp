#include "lr_parsing_model.h"
#include "yaml_cfg_loader.h"
#include "visualization_helper.h"
#include "simple_lr_parsing_table_generator.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

// a simple frontend: arg 1 is the config file path, output its nfa/dfa/parsing table

int main(int argc, char *argv[])
{
    // check the number of arguments
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    // read the config file
    std::string config_file_path = argv[1];
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open())
    {
        std::cerr << "Error: Could not open config file: " << config_file_path << std::endl;
        return 1;
    }
    // get file name
    std::string file_name = config_file_path.substr(config_file_path.find_last_of("/\\") + 1);

    cfg_model::CFG cfg;
    YAML_CFG_Loader cfg_loader;
    cfg = cfg_loader.LoadCFG(config_file_path);
    config_file.close();
    // generate the parsing table
    SimpleLRParsingTableGenerator generator;
    lr_parsing_model::ItemSetNFAGenerationResult nfa_result = generator.generate_item_set_nfa(cfg);
    nfa_model::NFA nfa = nfa_result.nfa;
    // generate the NFA dot file, use the file name as the output file name
    std::string nfa_dot_file_path = file_name + ".nfa.dot";
    visualization_helper::generate_nfa_dot_file(nfa, nfa_dot_file_path, true);
    std::cout << "NFA dot file generated: " << nfa_dot_file_path << std::endl;
    // generate the DFA dot file
    lr_parsing_model::ItemSetDFAGenerationResult dfa_result = generator.generate_item_set_dfa(cfg);
    dfa_model::DFA<std::string> dfa = dfa_result.dfa;
    // generate the DFA dot file, use the file name as the output file name
    std::string dfa_dot_file_path = file_name + ".dfa.dot";
    visualization_helper::generate_dfa_dot_file(dfa, dfa_dot_file_path, true);
    std::cout << "DFA dot file generated: " << dfa_dot_file_path << std::endl;
    // generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // output the parsing table
    visualization_helper::pretty_print_parsing_table(parsing_table);
    std::cout << "Parsing table generated: " << file_name << ".parsing_table.txt" << std::endl;


    return 0;
}