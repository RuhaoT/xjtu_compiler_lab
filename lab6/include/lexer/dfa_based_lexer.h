#ifndef DFA_BASED_LEXER_H
#define DFA_BASED_LEXER_H

#include <string>
#include <vector>
#include <set>
#include <memory>
#include "dfa_model.h"
#include "lexer_interface.h"
#include "standard_dfa_simulator.h"
#include <unordered_set>

namespace DFABasedLexerHelper
{
    struct IntermediateState
{
    std::string token_type_name;  // Token type
    std::string token_value; // Token value
    int token_priority_level; // Token priority level
    int prefix_length;       // Length of the prefix(token value)

    // overload the < operator for sorting
    bool operator<(const IntermediateState &other) const
    {
        return prefix_length < other.prefix_length;
    }
};

struct SingleIterationResult
{
    bool has_matched; // Indicates if a match was found
    IntermediateState longest_prefix_state; // Longest prefix state
};
}

struct DFALexerSetup
{
    std::vector<TokenType> token_types; // List of token types
    std::vector<dfa_model::DFA<char>> dfa_configurations; // List of DFA configurations
};

// DFA-based lexer class
class DFABasedLexer : public LexerInterface
{
private:
    std::unordered_set<TokenType> token_types; // Set of token types

    std::unordered_map<std::string, std::unique_ptr<StandardDFASimulator>> dfa_simulators; // DFA simulators for each token type

public:
    DFABasedLexer(const DFALexerSetup &construction_info);
    ~DFABasedLexer();

    // parse string
    LexerResult Parse(const std::string &input) override;

    

private:
    // Check if a give string match a specific token type
    bool MatchTokenType(const std::string &input, const TokenType &token_type) const;

    std::vector<Token> ParseStringToTokens(const std::string &input) const;

    DFABasedLexerHelper::SingleIterationResult SingleIterationMatchLongestPrefix(
        const std::string &input) const;
};

#endif // !DFA_BASED_LEXER_H
