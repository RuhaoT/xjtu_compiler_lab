#include "gtest/gtest.h"
#include "testing_utils.h"
#include "spdlog/spdlog.h"
#include "flex_based_lexer.h"
#include "yaml_lexer_factory.h"
#include <fstream>

class FlexBasedLexerTest : public ::testing::Test {
protected:
    // when setting up the fixture, init the logger
    static void SetUpTestSuite() {
        // create a basic file logger, for now just store the logs in the current directory
        std::string log_filename = "flex_based_lexer_tests.log";
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

TEST_F(FlexBasedLexerTest, FlexBasedLexerTest_ConstructMinimalLexer_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ConstructMinimalLexer_Test #####");
    
    std::string config_file = "test/data/lexer/flex_based_lexer/minimal_correct_lexer_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // Create a lexer factory
    YAMLLexerFactory lexer_factory;
    // Create a lexer using the factory
    ASSERT_NO_THROW(lexer_factory.CreateLexer("FLEX", config_file, config_file));
}

// Basic successful parsing test
TEST_F(FlexBasedLexerTest, FlexBasedLexerTest_ParseStringToTokens_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ParseStringToTokens_Test #####");
    
    std::string config_file = "test/data/lexer/flex_based_lexer/minimal_correct_lexer_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    YAMLLexerFactory lexer_factory;
    auto lexer = lexer_factory.CreateLexer("FLEX", config_file, config_file);
    
    std::string input = "a bb ccc";
    LexerResult result = lexer->Parse(input);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.tokens.size(), 3);
}

// complicated parsing test
TEST_F(FlexBasedLexerTest, FlexBasedLexerTest_ParseStringToTokens_Complicated_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ParseStringToTokens_Complicated_Test #####");
    
    std::string config_file = "test/data/lexer/flex_based_lexer/sim_c_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    YAMLLexerFactory lexer_factory;
    auto lexer = lexer_factory.CreateLexer("FLEX", config_file, config_file);
    
    std::string input = "while \\(true\\) \\{int a\\=0\\;\\}";
    LexerResult result = lexer->Parse(input);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.tokens.size(), 11);
    spdlog::info("Parsed tokens:");
    for (const auto& token : result.tokens) {
        spdlog::info("Token type: {}, value: {}", token.type, token.value);
    }
}