#include "gtest/gtest.h"
#include "syntax_semantic_analyzer/scope_table.h"
#include "common/testing_utils.h"

// Fixture for ScopeTable tests
class ScopeTableTest : public ::testing::Test {
protected:
    ScopeTable table;
    std::string test_log_file = "scope_table_test.log";

    void SetUp() override {
        init_fixture_logger(test_log_file);
        add_test_start_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
        table.reset(); // Reset table before each test
    }

    void TearDown() override {
        add_test_end_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
        release_fixture_logger();
    }
};

// Test case for basic scope operations
TEST_F(ScopeTableTest, BasicScopeOperations) {
    EXPECT_EQ(table.getCurrentScope(), 0); // Initial scope

    table.enterNewScope();
    EXPECT_EQ(table.getCurrentScope(), 1);
    EXPECT_EQ(table.getDirectParentScope(1), 0);

    table.enterNewScope();
    EXPECT_EQ(table.getCurrentScope(), 2);
    EXPECT_EQ(table.getDirectParentScope(2), 1);

    table.exitCurrentScope();
    EXPECT_EQ(table.getCurrentScope(), 1);

    table.exitCurrentScope();
    EXPECT_EQ(table.getCurrentScope(), 0);

    // Test exiting global scope (should throw or handle gracefully depending on design)
    // For now, let's assume it's an error to exit global scope, or it does nothing.
    // Depending on your ScopeTable::exitCurrentScope() implementation, this might need adjustment.
    // If it throws, use ASSERT_THROW.
    // If it does nothing, this is fine.
    // table.exitCurrentScope(); 
    // EXPECT_EQ(table.getCurrentScope(), 0); 
}

