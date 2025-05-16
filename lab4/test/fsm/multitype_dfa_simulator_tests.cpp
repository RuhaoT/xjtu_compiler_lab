#include "gtest/gtest.h"
#include "multitype_dfa_simulator.h"
#include "yaml_dfa_config_frontend.h"
#include "testing_utils.h"
#include "spdlog/spdlog.h"
#include <fstream>

class MultiTypeDFASimulatorTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "multitype_dfa_simulator_tests.log";
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

TEST_F(MultiTypeDFASimulatorTests, TestUpdateDFA) {
    // Create a minimal DFA for testing
    std::string filename = "test/data/fsm/multitype_dfa_simulator/dfa_config_real.yml";

    // check if the file exists
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open()) << "File not found: " << filename;

    YAMLDFAConfigFrontend yaml_config;
    ASSERT_TRUE(yaml_config.LoadConfig(filename)) << "Failed to load config from " << filename;
    ASSERT_TRUE(yaml_config.CheckConfig()) << "Failed to check config from " << filename;
    dfa_model::DFA<char> dfa = yaml_config.ConstructDFA();
    
    MultiTypeDFASimulator<char> sim = MultiTypeDFASimulator<char>();
    ASSERT_TRUE(sim.UpdateDFA(dfa));
    ASSERT_TRUE(sim.SimulateString({'3', '.', '4', '5', '6'})) << "Failed to simulate string in simulator";
    ASSERT_FALSE(sim.SimulateString({'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1'}));
}