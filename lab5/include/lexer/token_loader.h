#ifndef TOKEN_LOADER_H
#define TOKEN_LOADER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "token_model.h"

class TokenLoader {
public:
    TokenLoader() = default;
    ~TokenLoader() = default;

    // Load tokens from a file
    void load_from_file(const std::string& filename);

    // load from token stream
    void load_from_tokens(const std::vector<Token>& tokens);

    // Get the loaded tokens
    const std::vector<Token>& get_tokens() const;

private:
    std::vector<Token> tokens; // Vector to store loaded tokens
};

#endif // !TOKEN_LOADER_H