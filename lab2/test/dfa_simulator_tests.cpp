#include "gtest/gtest.h"
#include "standard_dfa_simulator.h"
#include "dfa_model.h"

class DFASimulatorTest : public ::testing::Test {
protected:
    StandardDFASimulator standard_dfa_simulator;
    DFA minimalDFA;

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
    }
    void TearDown() override {
        // Clean up if necessary
    }
};

TEST_F(DFASimulatorTest, UpdateDFAValid) {
    ASSERT_TRUE(standard_dfa_simulator.UpdateDFA(minimalDFA));
    // Further check: maybe simulate a known string for minimalDFA
    std::string log;
    ASSERT_TRUE(standard_dfa_simulator.SimulateString("a", log)); // Assumes UpdateDFA worked
    ASSERT_FALSE(standard_dfa_simulator.SimulateString("c", log)); // Assumes "b" is not accepted
}