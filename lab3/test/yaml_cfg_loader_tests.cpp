#include "gtest/gtest.h"
#include "testing_utils.h"
#include "yaml_cfg_loader.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <fstream>
#include <memory>

// create a fixture for log handling
class YamlCfgLoaderTests : public ::testing::Test
{
    protected:

    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "yaml_cfg_loader_tests.log";
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

TEST_F(YamlCfgLoaderTests, LoadMinimalCorrectCFG)
{
    std::string filename = "test/data/yaml_cfg_loader/minimal_correct_cfg.yml";
    // check if the file exists
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open()) << "File not found: " << filename;
    // load the CFG from the YAML file
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckYAMLFile(filename));
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::ParseYAMLFile(filename));
}

TEST_F(YamlCfgLoaderTests, LoadAndCheckMinimalCorrectCFG)
{
    std::string filename = "test/data/yaml_cfg_loader/minimal_correct_cfg.yml";
    // check if the file exists
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open()) << "File not found: " << filename;
    // load the CFG from the YAML file
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckYAMLFile(filename));
    cfg_model::CFG cfg = YAML_CFG_Loader_Helper::ParseYAMLFile(filename);
    // check the validity of the CFG
    ASSERT_NO_THROW(YAML_CFG_Loader_Helper::CheckCFG(cfg));
}