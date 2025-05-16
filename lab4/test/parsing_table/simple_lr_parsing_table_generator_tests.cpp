#include "gtest/gtest.h"
#include "simple_lr_parsing_table_generator.h"
#include "testing_utils.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <string>

// test fixture for SimpleLRParsingTableGenerator
class SimpleLRParsingTableGeneratorTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "simple_lr_parsing_table_generator_tests.log";
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
TEST_F(SimpleLRParsingTableGeneratorTests, TestGenerateParsingTable)
{
    // Load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/simple_lr_parsing_table_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";
}

// Test generating a parsing table from a complicated CFG with conflicts
TEST_F(SimpleLRParsingTableGeneratorTests, TestGenerateParsingTableWithConflict)
{
    // Load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/simple_lr_parsing_table_generator/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = load_test_cfg(filename);

    // Create an instance of SimpleLRParsingTableGenerator
    SimpleLRParsingTableGenerator generator;

    // Generate the parsing table
    lr_parsing_model::LRParsingTable parsing_table = generator.generate_parsing_table(cfg);
    // Check the parsing table
    ASSERT_TRUE(parsing_table.filling_check()) << "Parsing table filling check failed";
    auto conflicts = parsing_table.find_conflicts();
    ASSERT_EQ(conflicts.size(), 2) << "Expected 2 conflicts in the parsing table";
}