#include "gtest/gtest.h"
#include "yaml_lexer_factory.h"
#include "dfa_based_lexer.h"
#include <spdlog/spdlog.h>
#include <fstream>

// test the constructor of DFABasedLexer
TEST(DFABasedLexerTest, DFABasedLexerTest_ConstructMinimalLexer_Test){
    spdlog::info("##### Entering DFABasedLexerTest_ConstructMinimalLexer_Test #####");
    std::string config_file = "test/data/dfa_based_lexer/minimal_correct_lexer_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    YAMLLexerFactory lexer_factory;
    ASSERT_NO_THROW(lexer_factory.CreateLexer("DFA", config_file, config_file));
}

// basic successful parsing test
TEST(DFABasedLexerTest, DFABasedLexerTest_ParseStringToTokens_Test){
    spdlog::info("##### Entering DFABasedLexerTest_ParseStringToTokens_Test #####");
    std::string config_file = "test/data/dfa_based_lexer/minimal_correct_lexer_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    YAMLLexerFactory lexer_factory;
    auto lexer = lexer_factory.CreateLexer("DFA", config_file, config_file);
    
    std::string input = "a bb ccc";
    LexerResult result = lexer->Parse(input);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.tokens.size(), 6);
}

// test the lexer with a complicated sim-c lexer config
TEST(DFABasedLexerTest, DFABasedLexerTest_ParseFileToTokens_Test){
    spdlog::info("##### Entering DFABasedLexerTest_ParseFileToTokens_Test #####");
    std::string config_file = "test/data/dfa_based_lexer/sim_c_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    YAMLLexerFactory lexer_factory;
    auto lexer = lexer_factory.CreateLexer("DFA", config_file, config_file);
    
    std::string input = "while \\(true\\) \\{int a\\=0\\;\\}";
    // std::string input = "while";
    LexerResult result = lexer->Parse(input);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.tokens.size(), 11);
    
    spdlog::info("Parsed tokens:");
    for (const auto& token : result.tokens) {
        spdlog::info("Token type: {}, value: {}", token.type, token.value);
    }
}