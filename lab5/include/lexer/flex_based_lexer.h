#ifndef FLEX_BASED_LEXER_H
#define FLEX_BASED_LEXER_H

#include "lexer_interface.h"
#include "reflex/pattern.h"
#include "reflex/matcher.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

struct FlexLexerSetup
{
    std::vector<TokenType> token_types; // List of token types
    std::vector<std::string> regex_patterns; // List of regex patterns
};

// auxiliary structs, exactly the same as in dfa_based_lexer.h
namespace FlexBasedLexerHelper
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

class FlexBasedLexer : public LexerInterface
{

private:
    std::unordered_set<TokenType> token_types; // Set of token types
    std::unordered_map<std::string, std::unique_ptr<reflex::Pattern>> regex_patterns; // Map of regex patterns

public:
    FlexBasedLexer(const FlexLexerSetup &construction_info);
    ~FlexBasedLexer() override = default;

    // parse string
    LexerResult Parse(const std::string &input) override;

private:
    // Check if a give string match a specific token type
    // this is the only difference from dfa_based_lexer
    // besides, these are nolonger const functions, because it needs to update the matcher
    bool MatchTokenType(const std::string &input, const TokenType &token_type);

    std::vector<Token> ParseStringToTokens(const std::string &input);

    FlexBasedLexerHelper::SingleIterationResult SingleIterationMatchLongestPrefix(
        const std::string &input);
};

#endif // !FLEX_BASED_LEXER_H

