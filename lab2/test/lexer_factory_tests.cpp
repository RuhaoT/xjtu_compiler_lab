#include "gtest/gtest.h"
#include "yaml_lexer_factory.h"
#include "dfa_model.h"
#include "lexer_interface.h"
#include <fstream>
#include <spdlog/spdlog.h>

TEST(YAMLLexerFactoryTest, LoadMinimalCorrectGeneralConfig) {
    spdlog::info("##### Entering LoadMinimalCorrectGeneralConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_correct_general_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    ASSERT_NO_THROW(LoadGeneralConfigs(config_file));
}

TEST(YAMLLexerFactoryTest, LoadAndCheckMinimalCorrectGeneralConfig) {
    spdlog::info("##### Entering LoadAndCheckMinimalCorrectGeneralConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_correct_general_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    ASSERT_NO_THROW(CheckTokenTypes(LoadGeneralConfigs(config_file)));
}

TEST(YAMLLexerFactoryTest, LoadAndCheckMinimalIncorrectGeneralConfig) {
    spdlog::info("##### Entering LoadAndCheckMinimalIncorrectGeneralConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_incorrect_general_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    // should load correctly, but checkTokenTypes should throw an exception
    ASSERT_NO_THROW(LoadGeneralConfigs(config_file));
    ASSERT_THROW(CheckTokenTypes(LoadGeneralConfigs(config_file)), std::runtime_error);
}

TEST(YAMLLexerFactoryTest, LoadMinimalCorrectDFABackendConfig) {
    spdlog::info("##### Entering LoadMinimalCorrectDFABackendConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_correct_dfa_backend_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    ASSERT_NO_THROW(LoadDFAConfigs(config_file));
}

TEST(YAMLLexerFactoryTest, LoadAndCheckMinimalCorrectDFABackendConfig) {
    spdlog::info("##### Entering LoadAndCheckMinimalCorrectDFABackendConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_correct_dfa_backend_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    ASSERT_NO_THROW(CheckDFAConfigurations(LoadDFAConfigs(config_file)));
}

TEST(YAMLLexerFactoryTest, LoadAndCheckMinimalIncorrectDFABackendConfig) {
    spdlog::info("##### Entering LoadAndCheckMinimalIncorrectDFABackendConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_incorrect_dfa_backend_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    // should load correctly, but checkDFAConfigurations should throw an exception
    ASSERT_NO_THROW(LoadDFAConfigs(config_file));
    ASSERT_THROW(CheckDFAConfigurations(LoadDFAConfigs(config_file)), std::runtime_error);
}

TEST(YAMLLexerFactoryTest, LoadAndCheckMinimalIncorrectGeneralAndDFABackendConfig) {
    spdlog::info("##### Entering LoadAndCheckMinimalIncorrectGeneralAndDFABackendConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_incorrect_general_and_dfa_backend_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    // should load correctly, and each config should pass its own check, but the relationship check should throw an exception
    ASSERT_NO_THROW(LoadGeneralConfigs(config_file));
    ASSERT_NO_THROW(LoadDFAConfigs(config_file));
    ASSERT_NO_THROW(CheckTokenTypes(LoadGeneralConfigs(config_file)));
    ASSERT_NO_THROW(CheckDFAConfigurations(LoadDFAConfigs(config_file)));
    ASSERT_THROW(CheckTokenDFARelationships(LoadGeneralConfigs(config_file), LoadDFAConfigs(config_file)), std::runtime_error);
}

TEST(YAMLLexerFactoryTest, LoadMinimalCorrectFlexBackendConfig) {
    spdlog::info("##### Entering LoadMinimalCorrectFlexBackendConfig #####");
    std::string config_file = "test/data/lexer_factory/minimal_correct_flex_backend_config.yml";
    // check if the file exist
    std::ifstream file(config_file);
    ASSERT_TRUE(file.good()) << "File " << config_file << " does not exist.";
    
    ASSERT_NO_THROW(LoadRegexConfigs(config_file));
}