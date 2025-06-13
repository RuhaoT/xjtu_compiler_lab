#include "gtest/gtest.h"
#include "lexer/token_loader.h"
#include "common/testing_utils.h" // For init_fixture_logger and release_fixture_logger

// Fixture for TokenLoader tests
class TokenLoaderTest : public ::testing::Test {
protected:
    TokenLoader loader;
    std::string test_data_dir = "test/data/lexer/token_loader/";

    static void SetUpTestSuite() {
        // Initialize the logger for the test suite
        std::string test_log_file = "token_loader_tests.log";
        init_fixture_logger(test_log_file);
    }

    static void TearDownTestSuite() {
        // Release the logger after all tests in the suite have run
        release_fixture_logger();
    }

    void SetUp() override {
        add_test_start_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

    void TearDown() override {
        add_test_end_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
    }
};

// Test case for loading a simple token file
TEST_F(TokenLoaderTest, LoadSimpleTokens) {
    // Create a dummy token file
    std::string test_file = test_data_dir + "emulated_file.txt";
    // clear and re-create the file for each test run
    std::ofstream outfile(test_file);
    outfile << "(IDENTIFIER, myVar)" << std::endl;
    outfile << "(OPERATOR, =)" << std::endl;
    outfile << "(INTEGER, 10)" << std::endl;
    outfile.close();

    loader.load_from_file(test_file);
    const auto& tokens = loader.get_tokens();

    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, "IDENTIFIER");
    EXPECT_EQ(tokens[0].value, "myVar");
    EXPECT_EQ(tokens[1].type, "OPERATOR");
    EXPECT_EQ(tokens[1].value, "=");
    EXPECT_EQ(tokens[2].type, "INTEGER");
    EXPECT_EQ(tokens[2].value, "10");
}

// Basic loading from file test
TEST_F(TokenLoaderTest, BasicLoadFromFileTest) {
    // Load tokens from a file
    TokenLoader loader;
    std::string test_data_path = test_data_dir + "simple_tokens.txt";
    
    ASSERT_NO_THROW(loader.load_from_file(test_data_path));

    // Check if tokens are loaded correctly
    const auto& tokens = loader.get_tokens();
    ASSERT_FALSE(tokens.empty());
    EXPECT_EQ(tokens.size(), 23);
    EXPECT_EQ(tokens[1].value, "b");
    EXPECT_EQ(tokens[3].value, "2");
    EXPECT_EQ(tokens[20].value, "2.8");
}

// loading from tokens test
TEST_F(TokenLoaderTest, LoadFromTokensTest) {
    // Create a vector of tokens
    std::vector<Token> tokens = {
        {"IDENTIFIER", "x"},
        {"OPERATOR", "+"},
        {"INTEGER", "5"}
    };

    // Load from tokens
    loader.load_from_tokens(tokens);
    
    // Check if tokens are loaded correctly
    const auto& loaded_tokens = loader.get_tokens();
    ASSERT_EQ(loaded_tokens.size(), 3);
    EXPECT_EQ(loaded_tokens[0].type, "IDENTIFIER");
    EXPECT_EQ(loaded_tokens[0].value, "x");
    EXPECT_EQ(loaded_tokens[1].type, "OPERATOR");
    EXPECT_EQ(loaded_tokens[1].value, "+");
    EXPECT_EQ(loaded_tokens[2].type, "INTEGER");
    EXPECT_EQ(loaded_tokens[2].value, "5");
}