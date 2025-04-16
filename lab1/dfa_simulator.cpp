#include <iostream>
#include <string>
#include <memory>
#include <set>
#include "include/yaml_config_frontend.h"
#include "include/standard_dfa_simulator.h"

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " <config_file> <mode> [argument]" << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  <config_file>    Path to the DFA configuration file (.yml format)" << std::endl;
    std::cout << "  <mode>           Operating mode: 'check' or 'generate'" << std::endl;
    std::cout << "    check          Check if the input string is accepted by the DFA" << std::endl;
    std::cout << "    generate       Generate all accepted strings with a maximum length" << std::endl;
    std::cout << "  [argument]       Depending on the mode:" << std::endl;
    std::cout << "    check mode:     The string to be checked" << std::endl;
    std::cout << "    generate mode:  Maximum length of the strings to generate (default is 5)" << std::endl;
}

int main(int argc, char* argv[]) {
    // Check basic number of parameters
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string config_file = argv[1];
    std::string mode = argv[2];
    
    // Verify operating mode
    if (mode != "check" && mode != "generate") {
        std::cerr << "Error: Mode must be 'check' or 'generate'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Verify parameters for check mode
    if (mode == "check" && argc < 4) {
        std::cerr << "Error: check mode requires a string to check" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Create YAML configuration frontend
    auto config_frontend = std::make_unique<YAMLConfigFrontend>();
    
    // Load configuration
    std::cout << "Loading DFA configuration from file: " << config_file << std::endl;
    if (!config_frontend->LoadConfig(config_file)) {
        std::cerr << "Failed to load DFA configuration from file: " << config_file << std::endl;
        return 1;
    }
    
    // Check DFA configuration
    std::cout << "Checking DFA configuration..." << std::endl;
    if (!config_frontend->CheckConfig()) {
        std::cerr << "DFA configuration is invalid." << std::endl;
        return 1;
    }
    
    std::cout << "DFA configuration is valid." << std::endl;
    
    // Construct DFA
    DFA dfa = config_frontend->ConstructDFA();
    
    // Create DFA simulator
    auto dfa_simulator = std::make_unique<StandardDFASimulator>();
    
    // Update DFA
    if (!dfa_simulator->UpdateDFA(dfa)) {
        std::cerr << "Failed to update DFA." << std::endl;
        return 1;
    }
    
    // Perform different operations depending on the mode
    if (mode == "check") {
        std::string input_string = argv[3];
        std::string simulation_log;
        
        std::cout << "Simulating string: '" << input_string << "'" << std::endl;
        bool is_accepted = dfa_simulator->SimulateString(input_string, simulation_log);
        
        // Print simulation log
        std::cout << simulation_log << std::endl;
        
        std::cout << "String '" << input_string << "' is " 
                 << (is_accepted ? "accepted" : "rejected") << " by the DFA." << std::endl;
    } else { // generate mode
        int max_length = 5; // Default value
        
        // If a length parameter is provided
        if (argc > 3) {
            try {
                max_length = std::stoi(argv[3]);
                if (max_length < 0) {
                    std::cerr << "Error: Length must be a non-negative integer" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to parse length parameter, using default value 5" << std::endl;
            }
        }
        
        std::cout << "Generating accepted strings of maximum length " << max_length << std::endl;
        
        auto accepted_strings = dfa_simulator->GenerateAcceptedStrings(max_length);
        
        std::cout << "Found " << accepted_strings.size() << " accepted strings:" << std::endl;
        for (const auto& str : accepted_strings) {
            std::cout << (str.empty() ? "(empty string)" : str) << std::endl;
        }
    }
    
    return 0;
}