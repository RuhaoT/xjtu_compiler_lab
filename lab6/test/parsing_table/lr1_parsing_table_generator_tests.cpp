#include "gtest/gtest.h"
#include "lr1_parsing_table_generator.h"
#include "visualization_helper.h"
#include "testing_utils.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <string>

// Test fixture for LR1ParsingTableGenerator
class LR1ParsingTableGeneratorTests : public ::testing::Test
{
protected:
    std::string test_data_dir = "test/data/parsing_table/lr1_parsing_table_generator/";
    // When setting up the fixture, init the logger
    static void SetUpTestSuite()
    {
        // Create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "lr1_parsing_table_generator_tests.log";
        // Init the logger
        LoggingEnvironment::logger = init_fixture_logger(log_filename);
    }

    // When tearing down the fixture, flush and drop the logger
    static void TearDownTestSuite()
    {
        release_fixture_logger();
    }

    // At the start of each test, log the test name
    void SetUp() override
    {
        // Separate line
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_start_log(test_name);
    }

    // At the end of each test, log the test name
    void TearDown() override
    {
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_end_log(test_name);
    }
};

// DFA generation test for LR(1) parsing table
TEST_F(LR1ParsingTableGeneratorTests, TestGenerateItemSetDFA)
{
    // Load the CFG from the YAML file
    std::string filename = test_data_dir + "minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of LR1ParsingTableGenerator
    LR1ParsingTableGenerator generator;

    // Generate the item set DFA
    lr_parsing_model::ItemSetDFAGenerationResult result = generator.generate_item_set_dfa(cfg);

    ASSERT_NO_THROW({
        // Check the DFA configuration
        dfa_model_helper::check_dfa_configuration<std::string>(result.dfa);
    }) << "DFA configuration check failed";

    visualization_helper::generate_dfa_dot_file(result.dfa, "minimal_correct_cfg_dfa", true);
}

// test complicated cfg 2
TEST_F(LR1ParsingTableGeneratorTests, TestGenerateItemSetDFAWithConflict)
{
    // Load the CFG from the YAML file
    std::string filename = test_data_dir + "complicated_cfg_2_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of LR1ParsingTableGenerator
    LR1ParsingTableGenerator generator;

    // Generate the item set DFA
    lr_parsing_model::ItemSetDFAGenerationResult result = generator.generate_item_set_dfa(cfg);

    ASSERT_NO_THROW({
        // Check the DFA configuration
        dfa_model_helper::check_dfa_configuration<std::string>(result.dfa);
    }) << "DFA configuration check failed";

    visualization_helper::generate_dfa_dot_file(result.dfa, "complicated_cfg_2_dfa", true);
}

// test for generating parsing table from a CFG
TEST_F(LR1ParsingTableGeneratorTests, TestGenerateParsingTable)
{
    // Load the CFG from the YAML file
    std::string filename = test_data_dir + "minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of LR1ParsingTableGenerator
    LR1ParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);

    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";

    // print the parsing table
    visualization_helper::pretty_print_parsing_table(parsing_table, true, "minimal_correct_cfg_parsing_table_lr1.md");
}

// test for generating parsing table from a complicated CFG with conflicts
TEST_F(LR1ParsingTableGeneratorTests, TestGenerateParsingTableWithConflict)
{
    // Load the CFG from the YAML file
    std::string filename = test_data_dir + "complicated_cfg_2_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of LR1ParsingTableGenerator
    LR1ParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);

    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";

    // print the parsing table
    visualization_helper::pretty_print_parsing_table(parsing_table, true, "complicated_cfg_2_parsing_table_lr1.md");
}

// test solving conflicts in a special SLR(1) non-solveable CFG
TEST_F(LR1ParsingTableGeneratorTests, TestResolveConflictsInSpecialCFG)
{
    // Load the CFG from the YAML file
    std::string filename = test_data_dir + "slr1_non_solveable_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of LR1ParsingTableGenerator
    LR1ParsingTableGenerator generator;

    // print the DFA
    lr_parsing_model::ItemSetDFAGenerationResult item_set_dfa_result = generator.generate_item_set_dfa(cfg);
    visualization_helper::generate_dfa_dot_file(item_set_dfa_result.dfa, "slr1_non_solveable_cfg_dfa", true);

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);

    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";

    // print the parsing table
    visualization_helper::pretty_print_parsing_table(parsing_table, true, "slr1_non_solveable_cfg_parsing_table_lr1.md");
}

// // test solving final_semantic_correction
// TEST_F(LR1ParsingTableGeneratorTests, TestResolveConflictsInFinalSemanticCorrection)
// {
//     // Load the CFG from the YAML file
//     std::string filename = test_data_dir + "final_semantic_correction.yml";
//     cfg_model::CFG cfg = load_test_cfg(filename);

//     // Create an instance of LR1ParsingTableGenerator
//     LR1ParsingTableGenerator generator;

//     // Generate the item set DFA
//     lr_parsing_model::ItemSetDFAGenerationResult item_set_dfa_result = generator.generate_item_set_dfa(cfg);
//     visualization_helper::generate_dfa_dot_file(item_set_dfa_result.dfa, "final_semantic_correction_dfa", true);

//     // Generate the parsing table
//     lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);

//     // Check the parsing table
//     ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";

//     // print the parsing table
//     visualization_helper::pretty_print_parsing_table(parsing_table, true, "final_semantic_correction_parsing_table_lr1.md");
// }