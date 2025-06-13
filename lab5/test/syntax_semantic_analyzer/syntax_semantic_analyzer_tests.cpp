#include "gtest/gtest.h"
#include "lexer/token_loader.h"
#include "common/testing_utils.h" // For init_fixture_logger and release_fixture_logger
#include "common/visualization_helper.h"
#include "syntax_semantic_analyzer/syntax_semantic_analyzer.h"
#include "syntax_semantic_analyzer/symbol_table.h"
#include "syntax_semantic_analyzer/semantic_loader.h"
#include "syntax_semantic_analyzer/syntax_semantic_model.h"
#include "cfg/cfg_model.h"
#include "parsing_table/simple_lr_parsing_table_generator.h"
#include "parsing_table/lr1_parsing_table_generator.h"
#include "spdlog/spdlog.h"

// Fixture for SyntaxSemanticAnalyzer tests
class SyntaxSemanticAnalyzerTest : public ::testing::Test {
protected:
    std::string test_data_dir = "test/data/syntax_semantic_analyzer/syntax_semantic_analyzer/";

    static void SetUpTestSuite() {
        // Initialize the logger for the test suite
        std::string test_log_file = "syntax_semantic_analyzer_tests.log";
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

// integration test for syntax semantic analyzer
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTest) {
    std::string cfg_semantic_file = test_data_dir + "final_semantic_correction.yml";

    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "simple_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load the CFG from a file
    std::string cfg_file_path = cfg_semantic_file;
    cfg_model::CFG cfg = load_test_cfg(cfg_file_path);

    // Temporarily set spdlog level to off to mute logs
    auto current_logger = get_fixture_logger();
    spdlog::level::level_enum old_level = current_logger->level();
    current_logger->set_level(spdlog::level::off);

    // Generate LR(0) parsing table
    SimpleLRParsingTableGenerator simple_lr_generator;
    lr_parsing_model::LRParsingTable simple_lr_parsing_table = simple_lr_generator.generate_parsing_table(cfg);
    // print parsing
    // visualization_helper::pretty_print_parsing_table(simple_lr_parsing_table, true, "debug_final_semantic_parsing_table.md");
    // print NFA/DFA
    visualization_helper::generate_dfa_dot_file(simple_lr_generator.generate_item_set_dfa(cfg).dfa, "debug_final_semantic_dfa", true);
    visualization_helper::generate_nfa_dot_file(simple_lr_generator.generate_item_set_nfa(cfg).nfa, "debug_final_semantic_nfa", true);

    // Generate the LR(1) parsing table
    LR1ParsingTableGenerator lr1_generator;
    lr_parsing_model::LRParsingTable slr1_parsing_table;

    slr1_parsing_table = lr1_generator.generate_parsing_table(cfg);

    // pretty print to file
    visualization_helper::pretty_print_parsing_table(slr1_parsing_table, true, "integration_final_semantic_parsing_table_lr1.md");
    // generate dfa dot file
    visualization_helper::generate_dfa_dot_file(lr1_generator.generate_item_set_dfa(cfg).dfa, "integration_final_semantic_dfa_lr1", true);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Restore the logger level
    current_logger->set_level(old_level);
    
    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(slr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is not empty
    ASSERT_FALSE(ast_tree.empty());

    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_final_semantic_ast_tree_blank", true);

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(slr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    
    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());
    // save the AST tree to a file
    visualization_helper::generate_ast_tree_dot_file(result.ast_tree, "integration_final_semantic_ast_tree_result", true);
    // save the symbol table to a file
    visualization_helper::pretty_print_symbol_table(result.symbol_table, true, "integration_final_semantic_symbol_table.md");

}