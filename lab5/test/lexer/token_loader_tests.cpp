#include "gtest/gtest.h"
#include "lexer/token_loader.h"
#include "common/testing_utils.h" // For init_fixture_logger and release_fixture_logger

// Fixture for TokenLoader tests
class TokenLoaderTest : public ::testing::Test {
protected:
    TokenLoader loader;


    static void SetUpTestSuite() {
        // Initialize the logger for the test suite
        std::string test_log_file = "token_loader_test.log";
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
    std::string test_file = "test/data/lexer/token_loader/simple_tokens.txt";
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

