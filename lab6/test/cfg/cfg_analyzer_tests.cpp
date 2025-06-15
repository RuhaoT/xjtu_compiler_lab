#include "gtest/gtest.h"
#include "testing_utils.h"
#include "cfg_analyzer.h"
#include "cfg_model.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include "lr_parsing_model.h"
#include "itemset_generator.h"

// create a fixture for log handling
class CFGAnalyzerTests : public ::testing::Test
{
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite()
    {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "cfg_analyzer_tests.log";
        // init the logger
        LoggingEnvironment::logger = init_fixture_logger(log_filename);
    }

    // when tearing down the fixture, flush and drop the logger
    static void TearDownTestSuite()
    {
        release_fixture_logger();
    }

    // at the start of each test, log the test name
    void SetUp() override
    {
        // separate line
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_start_log(test_name);
    }
    // at the end of each test, log the test name
    void TearDown() override
    {
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        add_test_end_log(test_name);
    }
};

// helper function to verify the first set
void verify_first_set(const cfg_model::FirstSet &first_set, const cfg_model::CFG &cfg, const std::string &file_path)
{
    // the expected first set is stored in the expected_first_sets node of the yaml file
    YAML::Node expected_first_sets = YAML::LoadFile(file_path)["expected_first_sets"];
    for (const auto &symbol_set : expected_first_sets)
    {
        std::string symbol_name = symbol_set["symbol"].as<std::string>();
        // find the symbol in the cfg
        cfg_model::symbol symbol;
        for (const auto &s : cfg.non_terminals)
        {
            if (s.name == symbol_name)
            {
                symbol = s;
                break;
            }
        }
        int expected_symbol_count = symbol_set["first_set"].size();
        bool has_epsilon = false;
        for (const auto &expected_symbol_name : symbol_set["first_set"])
        {
            cfg_model::symbol expected_symbol;
            if (expected_symbol_name.as<std::string>().length() == 0)
            {
                has_epsilon = true;
                // check if the symbol is in the epsilon production set
                ASSERT_TRUE(first_set.symbols_with_epsilon.count(symbol) > 0)
                    << "Symbol " << std::string(symbol) << " not found in epsilon production set";
                continue;
            }
            for (const auto &s : cfg.terminals)
            {
                if (s.name == expected_symbol_name.as<std::string>())
                {
                    expected_symbol = s;
                    break;
                }
            }
            // check if the expected symbol is in the first set
            ASSERT_TRUE(first_set.first_set.at(expected_symbol).count(expected_symbol) > 0)
                << "Expected symbol " << expected_symbol_name.as<std::string>() << " not found in first set of "
                << symbol_name;
        }
        // check if the expected symbol count is correct
        if (has_epsilon)
        {
            expected_symbol_count--;
        }
        ASSERT_EQ(first_set.first_set.at(symbol).size(), expected_symbol_count)
            << "Expected symbol count for " << symbol_name << " is incorrect";
    }
}

void verify_follow_set(const cfg_model::FollowSet &follow_set, const cfg_model::CFG &cfg, const std::string &file_path)
{
    // the expected follow set is stored in the expected_follow_sets node of the yaml file
    YAML::Node file = YAML::LoadFile(file_path);
    YAML::Node expected_follow_sets = file["expected_follow_sets"];
    std::string end_symbol_name = file["end_of_input"].as<std::string>();
    cfg_model::symbol end_symbol = follow_set.end_symbol;
    
    for (const auto &symbol_set : expected_follow_sets)
    {
        std::string symbol_name = symbol_set["symbol"].as<std::string>();
        // find the symbol in the cfg
        cfg_model::symbol symbol;
        for (const auto &s : cfg.non_terminals)
        {
            if (s.name == symbol_name)
            {
                symbol = s;
                break;
            }
        }
        int expected_symbol_count = symbol_set["follow_set"].size();
        for (const auto &expected_symbol_name : symbol_set["follow_set"])
        {
            cfg_model::symbol expected_symbol;
            if (expected_symbol_name.as<std::string>() == end_symbol_name)
            {
                expected_symbol = end_symbol;
                // check if the end symbol is in the follow set
                ASSERT_TRUE(follow_set.follow_set.at(symbol).count(expected_symbol) > 0)
                    << "Expected symbol " << expected_symbol_name.as<std::string>() << " not found in follow set of "
                    << symbol_name;
                continue;
            }
            for (const auto &s : cfg.terminals)
            {
                if (s.name == expected_symbol_name.as<std::string>())
                {
                    expected_symbol = s;
                    break;
                }
            }
            // check if the expected symbol is in the follow set
            ASSERT_TRUE(follow_set.follow_set.at(symbol).count(expected_symbol) > 0)
                << "Expected symbol " << expected_symbol_name.as<std::string>() << " not found in follow set of "
                << symbol_name;
        }
        // check if the expected symbol count is correct
        ASSERT_EQ(follow_set.follow_set.at(symbol).size(), expected_symbol_count)
            << "Expected symbol count for " << symbol_name << " is incorrect";
    }
}

// Test cases
TEST_F(CFGAnalyzerTests, TestComputeFirstSet)
{
    std::string filename = "test/data/cfg/cfg_analyzer/cfg_1.yml";

    cfg_model::CFG cfg = load_test_cfg(filename);
    cfg = itemset_generator_helper::expand_cfg(cfg);
    CFGAnalyzer cfg_analyzer(cfg);
    ASSERT_NO_THROW(cfg_analyzer.computeFirstSet());
    cfg_model::FirstSet first_set = cfg_analyzer.getFirstSet();
    verify_first_set(first_set, cfg, filename);
}

TEST_F(CFGAnalyzerTests, TestComputeFirstSet2)
{
    std::string filename = "test/data/cfg/cfg_analyzer/cfg_2.yml";

    cfg_model::CFG cfg = load_test_cfg(filename);
    cfg = itemset_generator_helper::expand_cfg(cfg);
    CFGAnalyzer cfg_analyzer(cfg);
    ASSERT_NO_THROW(cfg_analyzer.computeFirstSet());
    cfg_model::FirstSet first_set = cfg_analyzer.getFirstSet();
    verify_first_set(first_set, cfg, filename);
}

TEST_F(CFGAnalyzerTests, TestComputeFollowSet)
{
    std::string filename = "test/data/cfg/cfg_analyzer/cfg_1.yml";

    cfg_model::CFG cfg = load_test_cfg(filename);
    cfg = itemset_generator_helper::expand_cfg(cfg);
    CFGAnalyzer cfg_analyzer(cfg);
    cfg_analyzer.computeFirstSet();
    ASSERT_NO_THROW(cfg_analyzer.computeFollowSet());
    cfg_model::FollowSet follow_set = cfg_analyzer.getFollowSet();
    verify_follow_set(follow_set, cfg, filename);
}

TEST_F(CFGAnalyzerTests, TestComputeFollowSet2)
{
    std::string filename = "test/data/cfg/cfg_analyzer/cfg_2.yml";

    cfg_model::CFG cfg = load_test_cfg(filename);
    cfg = itemset_generator_helper::expand_cfg(cfg);
    CFGAnalyzer cfg_analyzer(cfg);
    cfg_analyzer.computeFirstSet();
    ASSERT_NO_THROW(cfg_analyzer.computeFollowSet());
    cfg_model::FollowSet follow_set = cfg_analyzer.getFollowSet();
    verify_follow_set(follow_set, cfg, filename);
}