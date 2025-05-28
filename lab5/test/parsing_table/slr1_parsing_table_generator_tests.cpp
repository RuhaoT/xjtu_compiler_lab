#include "gtest/gtest.h"
#include "slr1_parsing_table_generator.h"
#include "testing_utils.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <string>

// test fixture for SLR1ParsingTableGenerator
class SLR1ParsingTableGeneratorTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "slr1_parsing_table_generator_tests.log";
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
// Test case for generating a parsing table from a CFG
TEST_F(SLR1ParsingTableGeneratorTests, TestGenerateParsingTable)
{
    // Load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/slr1_parsing_table_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of SLR1ParsingTableGenerator
    SLR1ParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";
    ASSERT_EQ(parsing_table.find_conflicts().size(), 0) << "Parsing table has conflicts";
}

TEST_F(SLR1ParsingTableGeneratorTests, TestGenerateParsingTableAndResolveConflicts)
{
    // Load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/slr1_parsing_table_generator/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of SLR1ParsingTableGenerator
    SLR1ParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";
    ASSERT_EQ(parsing_table.find_conflicts().size(), 0) << "Parsing table has conflicts";
}

TEST_F(SLR1ParsingTableGeneratorTests, TestGenerateParsingTableAndResolveConflicts_2)
{
    // Load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/slr1_parsing_table_generator/homework_8_4_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of SLR1ParsingTableGenerator
    SLR1ParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";
    ASSERT_EQ(parsing_table.find_conflicts().size(), 0) << "Parsing table has conflicts";
}

