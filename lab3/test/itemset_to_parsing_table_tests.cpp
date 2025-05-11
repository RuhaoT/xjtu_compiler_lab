#include "gtest/gtest.h"
#include "testing_utils.h"
#include "itemset_to_parsing_table.h"
#include "cfg_model.h"
#include "lr_parsing_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "standard_nfa_ctdfa_converter.h"
#include "yaml_cfg_loader.h"
#include "itemset_generator.h"
#include "spdlog/spdlog.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// test fixture for ItemSetToParsingTable
class ItemSetToParsingTableTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "itemset_to_parsing_table_tests.log";
        // init the logger
        LoggingEnvironment::logger = init_fixture_logger(log_filename);
    }

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

TEST_F(ItemSetToParsingTableTest, TestItemSetToParsingTableConstructor)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/itemset_to_parsing_table/minimal_correct_cfg.yml";
    lr_parsing_model::ItemSet itemset = generate_test_itemset(cfg_file_path);
    // create the itemset to parsing table object
    ASSERT_NO_THROW({
        ItemSetToParsingTable itemset_to_parsing_table(itemset);
    }) << "ItemSetToParsingTable constructor threw an exception";
}

TEST_F(ItemSetToParsingTableTest, TestItemSetToParsingTableWithConflict)
{
    // Load the CFG from the YAML file
    std::string cfg_file_path = "test/data/itemset_to_parsing_table/minimal_correct_cfg_with_conflict.yml";
    lr_parsing_model::ItemSet itemset = generate_test_itemset(cfg_file_path);
    // create the itemset to parsing table object
    ASSERT_NO_THROW({
        ItemSetToParsingTable itemset_to_parsing_table(itemset);
    }) << "ItemSetToParsingTable constructor threw an exception";
}