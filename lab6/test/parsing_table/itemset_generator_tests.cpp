#include "gtest/gtest.h"
#include "itemset_generator.h"
#include "yaml_cfg_loader.h"
#include "testing_utils.h"
#include "spdlog/spdlog.h"

// test fixture for itemset generator
class ItemSetGeneratorTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "itemset_generator_tests.log";
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

TEST_F(ItemSetGeneratorTests, TestExpandCFG)
{
    // load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/itemset_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    int original_symbol_count = cfg.non_terminals.size() + cfg.terminals.size();
    // expand the CFG
    cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);
    // check the expanded CFG
    ASSERT_EQ(expanded_cfg.start_symbol.name, "X_expanded");
    ASSERT_EQ(expanded_cfg.production_rules.size(), cfg.production_rules.size() + 1);
    ASSERT_EQ(expanded_cfg.production_rules[expanded_cfg.start_symbol].size(), 1);
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckCFG(expanded_cfg));
    int expanded_symbol_count = expanded_cfg.non_terminals.size() + expanded_cfg.terminals.size();
    // check that the number of symbols has increased
    ASSERT_EQ(expanded_symbol_count, original_symbol_count + 2); // one expanded start symbol and one end symbol
}

TEST_F(ItemSetGeneratorTests, TestExpandCFGNameOccupied)
{
    // load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/itemset_generator/cfg_inital_symbol_expanded_name_occupied.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    // expand the CFG
    cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);
    // check the expanded CFG
    ASSERT_EQ(expanded_cfg.start_symbol.name, "S_expanded_expanded_expanded");
    ASSERT_EQ(expanded_cfg.production_rules.size(), cfg.production_rules.size() + 1);
    ASSERT_EQ(expanded_cfg.production_rules[expanded_cfg.start_symbol].size(), 1);
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckCFG(expanded_cfg));
}

TEST_F(ItemSetGeneratorTests, TestGenerateCFGItems)
{
    // load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/itemset_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    
    ItemSetGenerator item_set_generator;
    lr_parsing_model::ItemSet item_set;
    ASSERT_NO_THROW(item_set = item_set_generator.generate_item_set(cfg));
    
    ASSERT_EQ(item_set.items.size(), 9);// 7 items + 2 initial and final items
    ASSERT_EQ(item_set.symbol_set.size(),(cfg.terminals.size() + cfg.non_terminals.size() + 2));
}

TEST_F(ItemSetGeneratorTests, TestGenerateComplicatedCFGItems)
{
    // load the CFG from the YAML file
    std::string filename = "test/data/parsing_table/itemset_generator/complicated_cfg_1_with_conflict.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    
    ItemSetGenerator item_set_generator;
    lr_parsing_model::ItemSet item_set;
    ASSERT_NO_THROW(item_set = item_set_generator.generate_item_set(cfg));
    
    ASSERT_EQ(item_set.items.size(), 20);// 18 items + 2 initial and final items
}