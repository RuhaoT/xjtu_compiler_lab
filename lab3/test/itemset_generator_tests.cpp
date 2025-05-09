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
    std::string filename = "test/data/itemset_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    // expand the CFG
    cfg_model::CFG expanded_cfg = itemset_generator_helper::expand_cfg(cfg);
    // check the expanded CFG
    ASSERT_EQ(expanded_cfg.start_symbol.name, "S_expanded");
    ASSERT_EQ(expanded_cfg.production_rules.size(), cfg.production_rules.size() + 1);
    ASSERT_EQ(expanded_cfg.production_rules[expanded_cfg.start_symbol].size(), 1);
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckCFG(expanded_cfg));
}

TEST_F(ItemSetGeneratorTests, TestExpandCFGNameOccupied)
{
    // load the CFG from the YAML file
    std::string filename = "test/data/itemset_generator/cfg_inital_symbol_expanded_name_occupied.yml";
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
    std::string filename = "test/data/itemset_generator/minimal_correct_cfg.yml";
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    
    ItemSetGenerator item_set_generator;
    lr_parsing_model::ItemSet item_set;
    ASSERT_NO_THROW(item_set = item_set_generator.generate_item_set(cfg));
    
    ASSERT_EQ(item_set.items.size(), 23);// 21 items + 2 initial and final items
}