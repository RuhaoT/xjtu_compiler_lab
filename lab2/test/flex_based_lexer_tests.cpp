#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "flex_based_lexer.h"
#include "yaml_lexer_factory.h"
#include <fstream>

TEST(FlexBasedLexerTest, FlexBasedLexerTest_ConstructMinimalLexer_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ConstructMinimalLexer_Test #####");
    
    std::string config_file = "test/data/flex_based_lexer/minimal_correct_lexer_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    // Create a lexer factory
    YAMLLexerFactory lexer_factory;
    // Create a lexer using the factory
    ASSERT_NO_THROW(lexer_factory.CreateLexer("FLEX", config_file, config_file));
}

// Basic successful parsing test
TEST(FlexBasedLexerTest, FlexBasedLexerTest_ParseStringToTokens_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ParseStringToTokens_Test #####");
    
    std::string config_file = "test/data/flex_based_lexer/minimal_correct_lexer_config.yml";
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
TEST(FlexBasedLexerTest, FlexBasedLexerTest_ParseStringToTokens_Complicated_Test) {
    spdlog::info("##### Entering FlexBasedLexerTest_ParseStringToTokens_Complicated_Test #####");
    
    std::string config_file = "test/data/flex_based_lexer/sim_c_config.yml";
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