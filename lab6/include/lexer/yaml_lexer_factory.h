#ifndef YAML_LEXER_FACTORY_H
#define YAML_LEXER_FACTORY_H
#include "lexer_factory_interface.h"
#include "dfa_model.h"
#include "lexer_interface.h"
#include "dfa_based_lexer.h"

class YAMLLexerFactory : public LexerFactoryInterface {
private:
    std::unordered_set<TokenType> unchecked_token_types; // Set of token types
    std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>> unchecked_dfa_mapping; // DFA configurations
    std::unordered_map<std::string, std::string> unchecked_regex_mapping; // Regex configurations

public:
    YAMLLexerFactory();
    ~YAMLLexerFactory();
    std::unique_ptr<LexerInterface> CreateLexer(const std::string& lexer_type, const std::string& general_config, const std::string& specific_config) override;

};

// Helper functions
std::unordered_set<TokenType> LoadGeneralConfigs(const std::string& general_config);
std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>> LoadDFAConfigs(const std::string& specific_config);
std::unordered_map<std::string, std::string> LoadRegexConfigs(const std::string& specific_config);
// Check if the token types are valid
void CheckTokenTypes(const std::unordered_set<TokenType>& token_types);
// Check if the DFA configurations are valid
void CheckDFAConfigurations(const std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>>& dfa_mapping);
// Check if the token types and DFA configurations are related
void CheckTokenDFARelationships(const std::unordered_set<TokenType>& token_types, const std::unordered_map<std::string, std::unique_ptr<dfa_model::DFA<char>>>& dfa_mapping);
// Check if the regex configurations and token types are related
void CheckRegexTokenTypeRelationships(const std::unordered_set<TokenType>& token_types, const std::unordered_map<std::string, std::string>& regex_mapping);
// regex checking will be done in the lexer itself

#endif // !YAML_LEXER_FACTORY_H