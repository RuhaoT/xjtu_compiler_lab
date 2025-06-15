#ifndef TOKEN_MODEL_H
#define TOKEN_MODEL_H

#include <string>

struct Token {
    std::string type; // Token type
    std::string value; // Token value

    // overload the == operator for comparison
    bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }
};

// provide a hash function for Token
namespace std {
    template <>
    struct hash<Token> {
        size_t operator()(const Token &token) const {
            return hash<string>()(token.type) ^ hash<string>()(token.value);
        }
    };
} // namespace std

#endif // !TOKEN_MODEL_H