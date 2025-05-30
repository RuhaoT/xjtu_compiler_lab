#include "yaml_lexer_factory.h"
#include "dfa_based_lexer.h"
#include "flex_based_lexer.h"
#include "dfa_model.h"
#include <yaml-cpp/yaml.h>
#include <memory>
#include <spdlog/spdlog.h>

// constructor/destructor
YAMLLexerFactory::YAMLLexerFactory() = default;
YAMLLexerFactory::~YAMLLexerFactory() = default;

// CreateLexer implementation
std::unique_ptr<LexerInterface> YAMLLexerFactory::CreateLexer(const std::string& lexer_type, const std::string& general_config, const std::string& specific_config) {

    // Load general configurations
    unchecked_token_types = LoadGeneralConfigs(general_config);
    // check if the general configurations are valid
    try {
        CheckTokenTypes(unchecked_token_types);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Token types check failed: " + std::string(e.what()));
    }

    if (lexer_type == "DFA"){
        // Load DFA configurations
        unchecked_dfa_mapping = LoadDFAConfigs(specific_config);
        // check if the DFA configurations are valid
        try{
        CheckDFAConfigurations(unchecked_dfa_mapping);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("DFA configurations check failed: " + std::string(e.what()));
        }
        try {
        CheckTokenDFARelationships(unchecked_token_types, unchecked_dfa_mapping);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Token and DFA relationship check failed: " + std::string(e.what()));
        }
        // prepare construction info
        std::vector<TokenType> token_types;
        std::vector<dfa_model::DFA<char>> dfa_configurations;
        // fill the token types and DFA configurations with unchecked_token_types and unchecked_dfas_mapping, which are already checked
        for (const auto& token_type : unchecked_token_types) {
            token_types.push_back(token_type);
            // find the corresponding DFA configuration
            auto it = unchecked_dfa_mapping.find(token_type.name);
            if (it != unchecked_dfa_mapping.end()) {
                dfa_configurations.push_back(*(it->second));
            }
            else {
                throw std::runtime_error("DFA configuration not found for token type: " + token_type.name);
            }
        }
        DFALexerSetup construction_info{token_types, dfa_configurations};

        // create a new DFA-based lexer
        return std::make_unique<DFABasedLexer>(construction_info);
    }
    else if (lexer_type == "FLEX") {
        // Load regex configurations
        unchecked_regex_mapping = LoadRegexConfigs(specific_config);
        // check if the regex configurations are valid
        try {
            CheckRegexTokenTypeRelationships(unchecked_token_types, unchecked_regex_mapping);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Token and regex relationship check failed: " + std::string(e.what()));
        }
        // prepare construction info
        std::vector<TokenType> token_types;
        std::vector<std::string> regex_patterns;
        // fill the token types and regex patterns with unchecked_token_types and unchecked_regex_mapping, which are already checked
        for (const auto& token_type : unchecked_token_types) {
            token_types.push_back(token_type);
            // find the corresponding regex pattern
            auto it = unchecked_regex_mapping.find(token_type.name);
            if (it != unchecked_regex_mapping.end()) {
                regex_patterns.push_back(it->second);
            }
            else {
                throw std::runtime_error("Regex pattern not found for token type: " + token_type.name);
            }
        }
        FlexLexerSetup construction_info{token_types, regex_patterns};

        // create a new flex-based lexer
        return std::make_unique<FlexBasedLexer>(construction_info);
    }
    else {
        throw std::invalid_argument("Unsupported lexer type");
    }
};

// Helper functions
// Load general configurations
std::unordered_set<TokenType> LoadGeneralConfigs(const std::string& general_config) {
    // Load file
    try {
        spdlog::debug("Loading general configurations from file: {}", general_config);
        YAML::Node config = YAML::LoadFile(general_config);
        if (!config["token_types"]) {
            throw std::runtime_error("Missing token_types in general configuration");
        }
        std::unordered_set<TokenType> token_types;
        for (const auto& token_type : config["token_types"]) {
            // check for 'name', 'priority' and 'has_content' fields
            if (!token_type["name"] || !token_type["priority"] || !token_type["has_content"]) {
                throw std::runtime_error("Missing name, priority, or has_content in token_types");
            }
            // append the token type to the unchecked_token_types set
            TokenType type;
            type.name = token_type["name"].as<std::string>();
            type.priority_level = token_type["priority"].as<int>();
            type.has_content = token_type["has_content"].as<bool>();
            token_types.insert(type);
        }
        spdlog::debug("Loaded {} token types", token_types.size());
        return token_types;
    }
    catch (const YAML::Exception& e) {
            throw std::runtime_error("Failed during YAML parsing: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to load general configurations: " + std::string(e.what()));
    }
}

// Load DFA configurations
std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>> LoadDFAConfigs(const std::string& specific_config) {
    // Load file
    try {
        YAML::Node config = YAML::LoadFile(specific_config);
        if (!config["dfas"]) {
            throw std::runtime_error("Missing dfas in specific configuration");
        }
        std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>> dfa_mapping;

        // iterate through each DFA in the dfas array
        for (const auto& dfa : config["dfas"]) {
            // check for 'name' and 'states' fields
            if (!dfa["name"] || !dfa["character_set"] || !dfa["states_set"] || !dfa["initial_state"] || !dfa["accepting_states"] || !dfa["transitions"]) {
                throw std::runtime_error("Incomplete DFA configuration, check for name, character_set, states_set, initial_state, accepting_states and transitions");
            }

            // construct the DFA
            dfa_model::DFA<char> dfa_config;
            std::string dfa_name = dfa["name"].as<std::string>();
            
            // separate the character set string and add each character to the set
            std::string character_set = dfa["character_set"].as<std::string>();
            for (char c : character_set) {
                dfa_config.character_set.insert(c);
            }
            spdlog::debug("DFA {} has {} characters in its character set", dfa_name, dfa_config.character_set.size());
            // add each state in the states_set array to the set
            for (const auto& state : dfa["states_set"]) {
                dfa_config.states_set.insert(state.as<std::string>());
            }
            // add the initial state
            dfa_config.initial_state = dfa["initial_state"].as<std::string>();
            // add the accepting states
            for (const auto& accepting_state : dfa["accepting_states"]) {
                dfa_config.accepting_states.insert(accepting_state.as<std::string>());
            }
            // add the transitions
            // construct each transition
            for (const auto& transition : dfa["transitions"]) {
                if (!transition["from"] || !transition["to"] || !transition["character"]) {
                    throw std::runtime_error("Incomplete transition configuration, check for from, to and input");
                }
                // check if the from state in already in the key of the transition map
                if (dfa_config.transitions.find(transition["from"].as<std::string>()) == dfa_config.transitions.end()) {
                    dfa_config.transitions[transition["from"].as<std::string>()] = std::unordered_map<char, std::string>();
                }

                // for each character in the character string of the transition, add the transition to the DFA
                for (const char& c : transition["character"].as<std::string>()) {
                    // append a char-state pair to the transition map
                    dfa_config.transitions[transition["from"].as<std::string>()].insert(std::pair<char, std::string>(c, transition["to"].as<std::string>()));
                }
            }

            // append the DFA to the unchecked_dfa_mapping map
            // check if the DFA name is already in the map
            if (dfa_mapping.find(dfa_name) != dfa_mapping.end()) {
                throw std::runtime_error("DFA name already exists: " + dfa_name);
            }
            // create a new DFA object and add it to the map
            dfa_mapping[dfa_name] = std::make_unique<dfa_model::DFA<char>>(dfa_config);
        }

        return dfa_mapping;
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed during YAML parsing: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to load DFA configurations: " + std::string(e.what()));
    }
}

// Load regex configurations
std::unordered_map<std::string, std::string> LoadRegexConfigs(const std::string& specific_config) {
    // Load file
    try {
        YAML::Node config = YAML::LoadFile(specific_config);
        if (!config["regexps"]) {
            throw std::runtime_error("Missing regex in specific configuration");
        }
        std::unordered_map<std::string, std::string> regex_mapping;
        for (const auto& regex : config["regexps"]) {
            // check for 'name' and 'pattern' fields
            if (!regex["regexp"] || !regex["token_type"]) {
                throw std::runtime_error("Incomplete regex configuration, check for regexp and token_type");
            }
            // append the regex to the unchecked_regex_mapping map
            regex_mapping[regex["token_type"].as<std::string>()] = regex["regexp"].as<std::string>();
        }
        return regex_mapping;
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed during YAML parsing: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to load regex configurations: " + std::string(e.what()));
    }
}

void CheckTokenTypes(const std::unordered_set<TokenType>& token_types) {
    // Check for: 1. Invalid token type names (empty or whitespace), or invalid priority levels (negative or zero) 2. Duplicated token type names, or priority levels
    std::unordered_set<std::string> token_type_names;
    std::unordered_set<int> priority_levels;
    for (const auto& token_type : token_types) {
        spdlog::debug("Checking token type: {}", token_type.name);
        spdlog::debug("This token type has priority level: {}", token_type.priority_level);

        // Check for empty or whitespace names
        if (token_type.name.empty() || std::all_of(token_type.name.begin(), token_type.name.end(), ::isspace)) {
            spdlog::debug("Invalid token type name: {}", token_type.name);
            throw std::runtime_error("Invalid token type name: " + token_type.name);
        }
        // Check for negative or zero priority levels
        if (token_type.priority_level <= 0) {
            spdlog::debug("Invalid priority level for token type: {}", token_type.name);
            throw std::runtime_error("Invalid priority level for token type: " + token_type.name);
        }
        // Check for duplicates
        if (token_type_names.find(token_type.name) != token_type_names.end()) {
            spdlog::debug("Duplicate token type name: {}", token_type.name);
            throw std::runtime_error("Duplicate token type name: " + token_type.name);
        }
        if (priority_levels.find(token_type.priority_level) != priority_levels.end()) {
            spdlog::debug("Duplicate priority level: {}", token_type.priority_level);
            throw std::runtime_error("Duplicate priority level: " + std::to_string(token_type.priority_level));
        }
        // Add to sets
        token_type_names.insert(token_type.name);
        priority_levels.insert(token_type.priority_level);
        spdlog::debug("Token type name and priority level are cosidered valid");
        spdlog::debug("Now the name set has {} token types", token_type_names.size());
        spdlog::debug("Now the priority level set has {} token types", priority_levels.size());
    }
}

void CheckDFAConfigurations(const std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>>& dfa_mapping) {
    // iterate through each DFA in the mapping
    for (const auto& dfa_pair : dfa_mapping) {
        const std::string& dfa_name = dfa_pair.first;
        const dfa_model::DFA<char>& dfa = *dfa_pair.second;

        spdlog::debug("Checking DFA: {}", dfa_name);

        // Check for empty character set
        if (dfa.character_set.empty()) {
            spdlog::debug("DFA {} has an empty character set", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " has an empty character set");
        }

        // Check for empty states set
        if (dfa.states_set.empty()) {
            spdlog::debug("DFA {} has an empty states set", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " has an empty states set");
        }

        // Check for empty initial state
        if (dfa.initial_state.empty()) {
            spdlog::debug("DFA {} has an empty initial state", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " has an empty initial state");
        }

        // Check for empty accepting states
        if (dfa.accepting_states.empty()) {
            spdlog::debug("DFA {} has an empty accepting states set", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " has an empty accepting states set");
        }

        // check if the initial state is in the states set
        if (dfa.states_set.find(dfa.initial_state) == dfa.states_set.end()) {
            spdlog::debug("DFA {} has an initial state not in the states set", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " has an initial state not in the states set");
        }

        // check if the accepting states are in the states set
        for (const auto& accepting_state : dfa.accepting_states) {
            if (dfa.states_set.find(accepting_state) == dfa.states_set.end()) {
                spdlog::debug("DFA {} has an accepting state not in the states set: {}", dfa_name, accepting_state);
                throw std::runtime_error("DFA " + dfa_name + " has an accepting state not in the states set: " + accepting_state);
            }
        }

        // iterate through each transition
        for (const auto& transition_pair : dfa.transitions) {
            const std::string& from_state = transition_pair.first;
            const auto& transitions = transition_pair.second;

            // Check if the from state is in the states set
            if (dfa.states_set.find(from_state) == dfa.states_set.end()) {
                spdlog::debug("DFA {} has a transition from a state not in the states set: {}", dfa_name, from_state);
                throw std::runtime_error("DFA " + dfa_name + " has a transition from a state not in the states set: " + from_state);
            }

            // iterate through each character-state pair in the transitions
            for (const auto& char_state_pair : transitions) {
                char input_char = char_state_pair.first;
                const std::string& to_state = char_state_pair.second;

                // Check if the input character is in the character set
                if (dfa.character_set.find(input_char) == dfa.character_set.end()) {
                    spdlog::debug("DFA {} has a transition on a character not in the character set: {}", dfa_name, input_char);
                    throw std::runtime_error("DFA " + dfa_name + " has a transition on a character not in the character set: " + input_char);
                }

                // Check if the to state is in the states set
                if (dfa.states_set.find(to_state) == dfa.states_set.end()) {
                    spdlog::debug("DFA {} has a transition to a state not in the states set: {}", dfa_name, to_state);
                    throw std::runtime_error("DFA " + dfa_name + " has a transition to a state not in the states set: " + to_state);
                }
            }
        }
        spdlog::debug("DFA {} is valid", dfa_name);
    }
}

void CheckTokenDFARelationships(const std::unordered_set<TokenType>& token_types, const std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>>& dfa_mapping) {
    // check the bijection between token type names and DFA mapping keys
    std::unordered_set<std::string> dfa_names;
    for (const auto& dfa_pair : dfa_mapping) {
        dfa_names.insert(dfa_pair.first);
    }
    for (const auto& token_type : token_types) {
        // check if the token type name is in the DFA mapping keys
        if (dfa_names.find(token_type.name) == dfa_names.end()) {
            spdlog::debug("Token type {} does not have a corresponding DFA", token_type.name);
            throw std::runtime_error("Token type " + token_type.name + " does not have a corresponding DFA");
        }
    }
    // check if the DFA mapping keys are in the token type names
    for (const auto& dfa_pair : dfa_mapping) {
        const std::string& dfa_name = dfa_pair.first;
        // check if the DFA name is in the token type names
        bool found = false;
        for (const auto& token_type : token_types) {
            if (token_type.name == dfa_name) {
                found = true;
                break;
            }
        }
        if (!found) {
            spdlog::debug("DFA {} does not have a corresponding token type", dfa_name);
            throw std::runtime_error("DFA " + dfa_name + " does not have a corresponding token type");
        }
    }
    spdlog::debug("Token types and DFA mapping are valid");
}

void CheckRegexTokenTypeRelationships(const std::unordered_set<TokenType>& token_types, const std::unordered_map<std::string, std::string>& regex_mapping) {
    // check the bijection between token type names and regex mapping keys
    std::unordered_set<std::string> regex_names;
    for (const auto& regex_pair : regex_mapping) {
        regex_names.insert(regex_pair.first);
    }
    for (const auto& token_type : token_types) {
        // check if the token type name is in the regex mapping keys
        if (regex_names.find(token_type.name) == regex_names.end()) {
            spdlog::debug("Token type {} does not have a corresponding regex", token_type.name);
            throw std::runtime_error("Token type " + token_type.name + " does not have a corresponding regex");
        }
    }
    // check if the regex mapping keys are in the token type names
    for (const auto& regex_pair : regex_mapping) {
        const std::string& regex_name = regex_pair.first;
        // check if the regex name is in the token type names
        bool found = false;
        for (const auto& token_type : token_types) {
            if (token_type.name == regex_name) {
                found = true;
                break;
            }
        }
        if (!found) {
            spdlog::debug("Regex {} does not have a corresponding token type", regex_name);
            throw std::runtime_error("Regex " + regex_name + " does not have a corresponding token type");
        }
    }
    spdlog::debug("Token types and regex mapping are valid");
}