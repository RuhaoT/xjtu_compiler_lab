#ifndef LEXER_INTERFACE_H
#define LEXER_INTERFACE_H

#include <string>
#include <vector>
#include "token_model.h"

// data models
struct TokenType
{
    std::string name;   // Token type name
    int priority_level; // Priority level for sorting
    bool has_content; // Indicates if the token type has content

    // overload the == operator for comparison
    bool operator==(const TokenType &other) const
    {
        return name == other.name && priority_level == other.priority_level;
    }
};

// provide a hash function for TokenType
namespace std
{
    template <>
    struct hash<TokenType>
    {
        size_t operator()(const TokenType &token_type) const
        {
            return hash<string>()(token_type.name) ^ hash<int>()(token_type.priority_level);
        }
    };
} // namespace std

struct LexerResult {
    bool success; // Indicates if the lexing was successful
    std::vector<Token> tokens; // List of tokens
    std::string error; // Error message if any
};

class LexerInterface {
public:
    virtual ~LexerInterface() = default;

    // parse string
    virtual LexerResult Parse(const std::string& input) = 0;
};

#endif // !LEXER_INTERFACE_H