#include "gtest/gtest.h"
#include "yaml_lexer_factory.h"
#include "standard_dfa_simulator.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

TEST(DFAConfigTest, FloatTypeDFATest){
    spdlog::info("##### Entering FloatTypeDFATest #####");
    std::string config_file = "test/data/dfa_config/float_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_TRUE(result) << "Input: " << input << "Log: " << simulation_log;
    }

    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_FALSE(result) << "Input: " << input << "Log: " << simulation_log;
    }
}

TEST(DFAConfigTest, NumberTypeDFATest){
    spdlog::info("##### Entering NumberTypeDFATest #####");
    std::string config_file = "test/data/dfa_config/num_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_TRUE(result) << "Input: " << input << "Log: " << simulation_log;
    }

    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_FALSE(result) << "Input: " << input << "Log: " << simulation_log;
    }
}

TEST(DFAConfigTest, IDTypeDFATest){
    spdlog::info("##### Entering IDTypeDFATest #####");
    std::string config_file = "test/data/dfa_config/id_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_TRUE(result) << "Input: " << input << "Log: " << simulation_log;
    }
    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::string simulation_log;
        bool result = dfa_simulator.SimulateString(input, simulation_log);
        ASSERT_FALSE(result) << "Input: " << input << "Log: " << simulation_log;
    }
}