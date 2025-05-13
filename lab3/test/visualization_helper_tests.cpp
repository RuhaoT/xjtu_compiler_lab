#include "gtest/gtest.h"
#include "visualization_helper.h"
#include "yaml_cfg_loader.h"
#include "testing_utils.h"
#include "spdlog/spdlog.h"
#include "simple_lr_parsing_table_generator.h"
#include <iostream>

// fixture for visualization helper tests
class VisualizationHelperTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "visualization_helper_tests.log";
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

// Test case for pretty printing the parsing table
TEST_F(VisualizationHelperTests, TestPrettyPrintParsingTable)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    ASSERT_NO_THROW(parsing_table.filling_check()) << "Parsing table filling check failed";
    // Pretty print the parsing table
    ASSERT_NO_THROW(visualization_helper::pretty_print_parsing_table(parsing_table)) << "pretty_print_parsing_table threw an exception";
}

// Test pretty print complicated parsing table with conflicts
TEST_F(VisualizationHelperTests, TestPrettyPrintComplicatedParsingTable)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    ASSERT_NO_THROW(parsing_table.filling_check()) << "Parsing table filling check failed";
    // Pretty print the parsing table
    ASSERT_NO_THROW(visualization_helper::pretty_print_parsing_table(parsing_table)) << "pretty_print_parsing_table threw an exception";
}

// Test case for generating NFA dot file
TEST_F(VisualizationHelperTests, TestGenerateNfaDotFile)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::ItemSetNFAGenerationResult result = generator.generate_item_set_nfa(cfg);
    nfa_model::NFA nfa = result.nfa;
    // Generate the NFA dot file
    std::string dot_file_path = "test/data/visualization_helper/nfa.dot";
    ASSERT_NO_THROW(visualization_helper::generate_nfa_dot_file(nfa, dot_file_path, true)) << "generate_nfa_dot_file threw an exception";
}

// Test case for generating CTDFA dot file
TEST_F(VisualizationHelperTests, TestGenerateCTDfaDotFile)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::ItemSetDFAGenerationResult result = generator.generate_item_set_dfa(cfg);
    dfa_model::DFA<std::string> dfa = result.dfa;
    // Generate the CTDFA dot file
    std::string dot_file_path = "test/data/visualization_helper/dfa.dot";
    ASSERT_NO_THROW(visualization_helper::generate_dfa_dot_file(dfa, dot_file_path, true)) << "generate_dfa_dot_file threw an exception";
}

// Test case for generating NFA dot file with complicated CFG
TEST_F(VisualizationHelperTests, TestGenerateNfaDotFileComplicated)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::ItemSetNFAGenerationResult result = generator.generate_item_set_nfa(cfg);
    nfa_model::NFA nfa = result.nfa;
    // Generate the NFA dot file
    std::string dot_file_path = "test/data/visualization_helper/nfa_complicated.dot";
    ASSERT_NO_THROW(visualization_helper::generate_nfa_dot_file(nfa, dot_file_path, true)) << "generate_nfa_dot_file threw an exception";
}

// Test case for generating CTDFA dot file with complicated CFG
TEST_F(VisualizationHelperTests, TestGenerateCTDfaDotFileComplicated)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/visualization_helper/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;
    // Generate the parsing table
    lr_parsing_model::ItemSetDFAGenerationResult result = generator.generate_item_set_dfa(cfg);
    dfa_model::DFA<std::string> dfa = result.dfa;
    // Generate the CTDFA dot file
    std::string dot_file_path = "test/data/visualization_helper/dfa_complicated.dot";
    ASSERT_NO_THROW(visualization_helper::generate_dfa_dot_file(dfa, dot_file_path, true)) << "generate_dfa_dot_file threw an exception";
}