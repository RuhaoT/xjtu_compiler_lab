#include "gtest/gtest.h"
#include "syntax_semantic_analyzer/semantic_loader.h"
#include "cfg/cfg_model.h"
#include "common/testing_utils.h"

// Fixture for SemanticLoader tests
class SemanticLoaderTest : public ::testing::Test
{
protected:
    std::string test_data_dir = "test/data/syntax_semantic_analyzer/semantic_loader/";

    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "semantic_loader_tests.log";
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

// basic loading test
TEST_F(SemanticLoaderTest, BasicLoadTest) {
    // load final_semantic.yml, assert no exceptions thrown 
    cfg_model::CFG cfg = load_test_cfg(test_data_dir + "final_semantic.yml");
    ASSERT_NO_THROW(load_semantic_info(test_data_dir + "final_semantic.yml", cfg));
};

