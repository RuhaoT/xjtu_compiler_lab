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
#include "syntax_semantic_analyzer/interm_code_generator.h"
#include "spdlog/spdlog.h"

// Fixture for SyntaxSemanticAnalyzer tests
class SyntaxSemanticAnalyzerTest : public ::testing::Test
{
protected:
    static std::string test_data_dir;
    static std::string cfg_semantic_file;
    static cfg_model::CFG cfg;
    static lr_parsing_model::LRParsingTable lr1_parsing_table;

    static void SetUpTestSuite()
    {
        // Initialize the logger for the test suite
        std::string test_log_file = "syntax_semantic_analyzer_tests.log";
        init_fixture_logger(test_log_file);
        // Temporarily set spdlog level to off to mute logs
        spdlog::level::level_enum old_level = get_fixture_logger()->level();
        set_fixture_logger_level(spdlog::level::off);

        // Load the CFG from a file
        std::string cfg_file_path = cfg_semantic_file;
        cfg = load_test_cfg(cfg_file_path);

        // Generate the LR(1) parsing table
        LR1ParsingTableGenerator lr1_generator;

        lr1_parsing_table = lr1_generator.generate_parsing_table(cfg);

        // pretty print to file
        visualization_helper::pretty_print_parsing_table(lr1_parsing_table, true, "integration_final_semantic_parsing_table_lr1.md");
        // generate dfa dot file
        visualization_helper::generate_dfa_dot_file(lr1_generator.generate_item_set_dfa(cfg).dfa, "integration_final_semantic_dfa_lr1", true);

        // Restore the logger level
        set_fixture_logger_level(old_level);
    }

    static void TearDownTestSuite()
    {
        // Release the logger after all tests in the suite have run
        release_fixture_logger();
    }

    void SetUp() override
    {
        add_test_start_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

    void TearDown() override
    {
        add_test_end_log(::testing::UnitTest::GetInstance()->current_test_info()->name());
    }
};

// initialize the static member variable
std::string SyntaxSemanticAnalyzerTest::test_data_dir = "test/data/syntax_semantic_analyzer/syntax_semantic_analyzer/";
std::string SyntaxSemanticAnalyzerTest::cfg_semantic_file = SyntaxSemanticAnalyzerTest::test_data_dir + "final_semantic_correction.yml";
cfg_model::CFG SyntaxSemanticAnalyzerTest::cfg;
lr_parsing_model::LRParsingTable SyntaxSemanticAnalyzerTest::lr1_parsing_table;

// integration test for syntax semantic analyzer, testing a minimal syntax correct but semantic incorrect case: missing main function
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestMinimalSemanticIncorrectMissMain)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "minimal_semantic_incorrect_missMain_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is not empty
    ASSERT_FALSE(ast_tree.empty());

    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_minimal_semantic_incorrect_missMain_ast_tree_blank", true);

    // perform syntax and semantic analysis, this should throw an exception due to semantic error
    try {
        syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());
        FAIL() << "Expected a semantic error to be thrown, but none was thrown.";
    } catch (const std::runtime_error& e) {
        // Check if the exception message contains the expected semantic error
        ASSERT_TRUE(std::string(e.what()).find("Main function not found") != std::string::npos)
            << "Unexpected exception message: " << e.what();
        spdlog::info("Caught expected semantic error: {}", e.what());
    }
}

// test a minimal correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestMinimalCorrect)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "minimal_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is not empty
    ASSERT_FALSE(ast_tree.empty());

    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_minimal_correct_ast_tree_blank", true);

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());
    // save the AST tree to a file
    visualization_helper::generate_ast_tree_dot_file(result.ast_tree, "integration_minimal_correct_ast_tree_result", true);

    // save the symbol table to a file
    visualization_helper::pretty_print_symbol_table(result.symbol_table, true, "integration_minimal_correct_symbol_table.md");
}

// test minima_multifunc_correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestMinimalMultiFuncCorrect)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "minimal_multifunc_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is
    ASSERT_FALSE(ast_tree.empty());
    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_minimal_multifunc_correct_ast_tree_blank", true);
    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());
    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());
    // save the AST tree to a file
    visualization_helper::generate_ast_tree_dot_file(result.ast_tree, "integration_minimal_multifunc_correct_ast_tree_result", true);
    // save the symbol table to a file
    visualization_helper::pretty_print_symbol_table(result.symbol_table, true, "integration_minimal_multifunc_correct_symbol_table.md");
}

// integration test for syntax semantic analyzer, testing a simple correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestSimpleCorrect)
{

    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "simple_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is not empty
    ASSERT_FALSE(ast_tree.empty());

    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_simple_correct_ast_tree_blank", true);

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());
    // save the AST tree to a file
    visualization_helper::generate_ast_tree_dot_file(result.ast_tree, "integration_simple_correct_ast_tree_result", true);
    // save the symbol table to a file
    visualization_helper::pretty_print_symbol_table(result.symbol_table, true, "integration_simple_correct_symbol_table.md");
}

// test generate intermediate code for a minimal correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestMinimalCorrectGenerateIntermediateCode)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "minimal_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());

    // generate intermediate code for the AST tree
    IntermCodeGenerator interm_code_generator(result.symbol_table, result.ast_tree);
    interm_code_generator.produce_intermediate_code("integration_minimal_correct_intermediate_code.txt");
}

// test generate intermediate code for a minimal multi-function correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestMinimalMultiFuncCorrectGenerateIntermediateCode)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "minimal_multifunc_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());

    // generate intermediate code for the AST tree
    IntermCodeGenerator interm_code_generator(result.symbol_table, result.ast_tree);
    interm_code_generator.produce_intermediate_code("integration_minimal_multifunc_correct_intermediate_code.txt");
}

// test generate intermediate code for a simple correct case
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestSimpleCorrectGenerateIntermediateCode)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "simple_correct_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());

    // generate intermediate code for the AST tree
    IntermCodeGenerator interm_code_generator(result.symbol_table, result.ast_tree);
    interm_code_generator.produce_intermediate_code("integration_simple_correct_intermediate_code.txt");
}

// test complicated tokens
TEST_F(SyntaxSemanticAnalyzerTest, IntegrationTestComplicatedTokens)
{
    // Create a token loader and load tokens from a file
    TokenLoader token_loader;
    std::string token_file_path = test_data_dir + "complicated_tokens.txt";
    token_loader.load_from_file(token_file_path);

    // Load semantic information
    std::string semantic_info_file = cfg_semantic_file;
    syntax_semantic_model::ProductionInfoMapping production_info_mapping = load_semantic_info(semantic_info_file, cfg);

    // Create an instance of SyntaxSemanticAnalyzer
    SyntaxSemanticAnalyzer analyzer;
    // Perform analysis
    analyzer.prepair_new_analysis(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    auto ast_tree = analyzer.get_blank_ast_tree();

    // Check if the AST tree is not empty
    ASSERT_FALSE(ast_tree.empty());

    // generate AST tree dot file
    visualization_helper::generate_ast_tree_dot_file(ast_tree, "integration_complicated_tokens_ast_tree_blank", true);

    // perform syntax and semantic analysis
    syntax_semantic_analyzer::analysis_result result = analyzer.analyze_syntax_semantics(lr1_parsing_table, production_info_mapping, token_loader.get_tokens());

    // Check if the result contains a valid AST tree
    ASSERT_FALSE(result.ast_tree.empty());
    // save the AST tree to a file
    visualization_helper::generate_ast_tree_dot_file(result.ast_tree, "integration_complicated_tokens_ast_tree_result", true);

    // save the symbol table to a file
    visualization_helper::pretty_print_symbol_table(result.symbol_table, true, "integration_complicated_tokens_symbol_table.md");

    // generate intermediate code for the AST tree
    IntermCodeGenerator interm_code_generator(result.symbol_table, result.ast_tree);
    interm_code_generator.produce_intermediate_code("integration_complicated_tokens_intermediate_code.txt");
}