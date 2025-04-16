#include "include/yaml_config_frontend.h"
#include <iostream>
#include <fstream>

YAMLConfigFrontend::YAMLConfigFrontend() : config(nullptr), is_loaded(false), is_checked(false) {}

YAMLConfigFrontend::~YAMLConfigFrontend() = default;

bool YAMLConfigFrontend::LoadConfig(const std::string& filepath) {
    try {
        // 尝试打开文件
        std::ifstream file(filepath);
        if (!file.good()) {
            std::cerr << "Error: Cannot open file " << filepath << std::endl;
            return false;
        }
        
        // 加载YAML配置
        config = std::make_unique<YAML::Node>(YAML::LoadFile(filepath));
        is_loaded = true;
        is_checked = false; // 重置检查状态
        
        // 解析DFA组件
        if (!ParseCharacterSet(*config) || 
            !ParseStatesSet(*config) ||
            !ParseInitialState(*config) ||
            !ParseAcceptingStates(*config) ||
            !ParseTransitions(*config)) {
            std::cerr << "Error: Failed to parse DFA configuration" << std::endl;
            return false;
        }
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfigFrontend::CheckConfig() const {
    if (!is_loaded) {
        std::cerr << "Error: Configuration not loaded" << std::endl;
        return false;
    }
    
    // 检查开始状态是否唯一并包含在状态集中
    if (dfa.initial_state.empty()) {
        std::cerr << "Error: Initial state is not defined" << std::endl;
        return false;
    }
    
    if (dfa.states_set.find(dfa.initial_state) == dfa.states_set.end()) {
        std::cerr << "Error: Initial state '" << dfa.initial_state 
                  << "' is not in the states set" << std::endl;
        return false;
    }
    
    // 检查接受状态集是否为空并包含在状态集中
    if (dfa.accepting_states.empty()) {
        std::cerr << "Error: Accepting states set is empty" << std::endl;
        return false;
    }
    
    for (const auto& state : dfa.accepting_states) {
        if (dfa.states_set.find(state) == dfa.states_set.end()) {
            std::cerr << "Error: Accepting state '" << state 
                      << "' is not in the states set" << std::endl;
            return false;
        }
    }
    
    // 检查转换表中的状态是否都存在于状态集中
    for (const auto& [from_state, transitions] : dfa.transitions) {
        if (dfa.states_set.find(from_state) == dfa.states_set.end()) {
            std::cerr << "Error: Transition from state '" << from_state 
                      << "' which is not in the states set" << std::endl;
            return false;
        }
        
        for (const auto& [input_char, to_state] : transitions) {
            if (dfa.character_set.find(input_char) == dfa.character_set.end()) {
                std::cerr << "Error: Transition on character '" << input_char 
                          << "' which is not in the character set" << std::endl;
                return false;
            }
            
            if (dfa.states_set.find(to_state) == dfa.states_set.end()) {
                std::cerr << "Error: Transition to state '" << to_state 
                          << "' which is not in the states set" << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

DFA YAMLConfigFrontend::ConstructDFA() const {
    if (!is_loaded) {
        std::cerr << "Error: Configuration not loaded" << std::endl;
        return DFA();
    }
    
    return dfa;
}

bool YAMLConfigFrontend::ParseCharacterSet(const YAML::Node& node) {
    if (!node["character_set"] || !node["character_set"].IsSequence()) {
        std::cerr << "Error: Missing or invalid character_set" << std::endl;
        return false;
    }
    
    dfa.character_set.clear();
    for (const auto& item : node["character_set"]) {
        std::string str = item.as<std::string>();
        if (str.length() == 1) {
            dfa.character_set.insert(str[0]);
        } else {
            std::cerr << "Error: Character '" << str << "' must be a single character" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool YAMLConfigFrontend::ParseStatesSet(const YAML::Node& node) {
    if (!node["states_set"] || !node["states_set"].IsSequence()) {
        std::cerr << "Error: Missing or invalid states_set" << std::endl;
        return false;
    }
    
    dfa.states_set.clear();
    for (const auto& item : node["states_set"]) {
        dfa.states_set.insert(item.as<std::string>());
    }
    
    return true;
}

bool YAMLConfigFrontend::ParseInitialState(const YAML::Node& node) {
    if (!node["initial_state"] || !node["initial_state"].IsScalar()) {
        std::cerr << "Error: Missing or invalid initial_state" << std::endl;
        return false;
    }
    
    dfa.initial_state = node["initial_state"].as<std::string>();
    return true;
}

bool YAMLConfigFrontend::ParseAcceptingStates(const YAML::Node& node) {
    if (!node["accepting_states"] || !node["accepting_states"].IsSequence()) {
        std::cerr << "Error: Missing or invalid accepting_states" << std::endl;
        return false;
    }
    
    dfa.accepting_states.clear();
    for (const auto& item : node["accepting_states"]) {
        dfa.accepting_states.insert(item.as<std::string>());
    }
    
    return true;
}

bool YAMLConfigFrontend::ParseTransitions(const YAML::Node& node) {
    if (!node["transitions"] || !node["transitions"].IsMap()) {
        std::cerr << "Error: Missing or invalid transitions" << std::endl;
        return false;
    }
    
    dfa.transitions.clear();
    
    for (const auto& state_pair : node["transitions"]) {
        std::string from_state = state_pair.first.as<std::string>();
        
        if (!state_pair.second.IsMap()) {
            std::cerr << "Error: Invalid transitions for state " << from_state << std::endl;
            return false;
        }
        
        for (const auto& transition : state_pair.second) {
            std::string input_str = transition.first.as<std::string>();
            std::string to_state = transition.second.as<std::string>();
            
            if (input_str.length() != 1) {
                std::cerr << "Error: Input symbol '" << input_str 
                          << "' must be a single character" << std::endl;
                return false;
            }
            
            char input_char = input_str[0];
            dfa.transitions[from_state][input_char] = to_state;
        }
    }
    
    return true;
}