#include "gtest/gtest.h"
#include "syntax_semantic_analyzer/symbol_table.h"
#include "syntax_semantic_analyzer/syntax_semantic_model.h" // For SymbolEntry
#include "common/testing_utils.h"

// Fixture for SymbolTable tests
class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTable table;
    std::string test_log_file = "symbol_table_test.log";

    void SetUp() override {
        init_fixture_logger(test_log_file);
        add_test_start_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
        // table.reset(); // If your symbol table has a reset/clear method
    }

    void TearDown() override {
        add_test_end_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
        release_fixture_logger();
    }
};

// Test case for adding and finding a symbol
TEST_F(SymbolTableTest, AddAndFindSymbol) {
    syntax_semantic_model::SymbolEntry entry;
    entry.symbol_name = "myVar";
    entry.symbol_type = syntax_semantic_model::SymbolType::Variable;
    entry.scope_id = 0;
    entry.data_type = "int";

    table.addSymbol(entry);

    EXPECT_TRUE(table.symbolExists("myVar", 0));
    auto found_symbol = table.findSymbolInScope("myVar", 0);
    ASSERT_TRUE(found_symbol.has_value());
    EXPECT_EQ(found_symbol->symbol_name, "myVar");
    EXPECT_EQ(found_symbol->data_type, "int");

    EXPECT_FALSE(table.symbolExists("nonExistentVar", 0));
    EXPECT_FALSE(table.findSymbolInScope("nonExistentVar", 0).has_value());
}

