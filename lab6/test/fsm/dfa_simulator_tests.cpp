#include "gtest/gtest.h"
#include "testing_utils.h"
#include "standard_dfa_simulator.h"
#include "dfa_model.h"

class DFASimulatorTest : public ::testing::Test {
protected:
    StandardDFASimulator standard_dfa_simulator;
    dfa_model::DFA<char> minimalDFA;

    static void SetUpTestSuite() {
        // Create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "dfa_simulator_tests.log";
        // init the logger
        LoggingEnvironment::logger = init_fixture_logger(log_filename);
    }
    static void TearDownTestSuite() {
        // Flush and drop the logger
        release_fixture_logger();
    }

    void SetUp() override {
        // Initialize a minimal DFA for testing
        minimalDFA.character_set = {'a', 'b'};
        minimalDFA.states_set = {"q0", "q1"};
        minimalDFA.initial_state = "q0";
        minimalDFA.accepting_states = {"q1"};
        minimalDFA.transitions = {
            {"q0", {{'a', "q1"}, {'b', "q0"}}},
            {"q1", {{'a', "q1"}, {'b', "q0"}}}
        };
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_start_log(test_name);

    }
    void TearDown() override {
        // Clean up if necessary
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_end_log(test_name);
    }
};

TEST_F(DFASimulatorTest, UpdateDFAValid) {
    ASSERT_TRUE(standard_dfa_simulator.UpdateDFA(minimalDFA));
    // Further check: maybe simulate a known string for minimalDFA
    std::vector<char> acceptable = {'a'};
    std::vector<char> unacceptable = {'c'};
    ASSERT_TRUE(standard_dfa_simulator.SimulateString(acceptable)); // Assumes UpdateDFA worked
    ASSERT_FALSE(standard_dfa_simulator.SimulateString(unacceptable)); // Assumes "b" is not accepted
}