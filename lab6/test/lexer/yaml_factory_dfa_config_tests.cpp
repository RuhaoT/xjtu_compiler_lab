#include "gtest/gtest.h"
#include "testing_utils.h"
#include "yaml_lexer_factory.h"
#include "standard_dfa_simulator.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

class DFAConfigTest : public ::testing::Test {
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "dfa_config_tests.log";
        // init the logger
        LoggingEnvironment::logger = init_fixture_logger(log_filename);
    }

    // when tearing down the fixture, flush and drop the logger
    static void TearDownTestSuite() {
        release_fixture_logger();
    }

    // at the start of each test, log the test name
    void SetUp() override {
        // separate line
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_start_log(test_name);
    }
    
    // at the end of each test, log the test name
    void TearDown() override {
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_end_log(test_name);
    }
};

TEST_F(DFAConfigTest, FloatTypeDFATest){
    spdlog::info("##### Entering FloatTypeDFATest #####");
    std::string config_file = "test/data/lexer/yaml_factory_dfa_config/float_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    dfa_model::DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        std::string simulation_log;
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_TRUE(result);
    }

    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        std::string simulation_log;
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_FALSE(result) << "Input: " << input << "Log: " << simulation_log;
    }
}

TEST_F(DFAConfigTest, NumberTypeDFATest){
    spdlog::info("##### Entering NumberTypeDFATest #####");
    std::string config_file = "test/data/lexer/yaml_factory_dfa_config/num_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    dfa_model::DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_TRUE(result);
    }

    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_FALSE(result);
    }
}

TEST_F(DFAConfigTest, IDTypeDFATest){
    spdlog::info("##### Entering IDTypeDFATest #####");
    std::string config_file = "test/data/lexer/yaml_factory_dfa_config/id_dfa_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // load yaml file
    YAML::Node config = YAML::LoadFile(config_file);
    
    auto dfa_config = LoadDFAConfigs(config_file);
    ASSERT_NO_THROW(CheckDFAConfigurations(dfa_config));
    // get the first DFA
    dfa_model::DFA dfa = *dfa_config.begin()->second;

    StandardDFASimulator dfa_simulator;
    ASSERT_TRUE(dfa_simulator.UpdateDFA(dfa));

    // test match cases
    for(const auto& test_case : config["test_cases"]["acceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_TRUE(result);
    }
    // test unmatch cases
    for(const auto& test_case : config["test_cases"]["unacceptable"]) {
        std::string input = test_case.as<std::string>();
        spdlog::info("Testing input: {}", input);
        std::vector<char> input_vector(input.begin(), input.end());
        bool result = dfa_simulator.SimulateString(input_vector);
        ASSERT_FALSE(result);
    }
}