#include "token_loader.h"
#include "spdlog/spdlog.h"
#include <stdexcept>

void TokenLoader::load_from_file(const std::string& filename) {
    try{
        // 1. check if the file exists
        std::ifstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Could not open file: {}", filename);
            throw std::runtime_error("File not found or could not be opened.");
        }
        // 2. prepare to read tokens
        std::vector<Token> loaded_tokens;
        std::string line;
        // 3. read the file line by line
        while (std::getline(file, line)) {
            if (line.length() < 4) { // Minimum like (a,b) actually (a,) is 4, (,,) is also 4
                spdlog::warn("Line too short to be a token: {}", line);
                continue;
            }

            if (line.front() != '(' || line.back() != ')') {
                spdlog::warn("Malformed line (missing parentheses): {}", line);
                continue;
            }

            // Remove parentheses
            std::string content = line.substr(1, line.length() - 2);
            
            size_t comma_pos = content.find(',');
            if (comma_pos == std::string::npos) {
                spdlog::warn("Malformed line (missing comma): {}", line);
                continue;
            }

            std::string type_str = content.substr(0, comma_pos);
            std::string value_str = content.substr(comma_pos + 1);

            // Trim leading space from value_str if present, as per "(TYPE, VALUE)"
            if (!value_str.empty() && value_str.front() == ' ') {
                value_str.erase(0, 1);
            }
            
            // Trim potential trailing spaces from type_str and value_str if necessary
            auto trim_spaces = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
                s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
            };
            
            trim_spaces(type_str);
            // Value string already had leading space trimmed, ensure trailing is also handled if any
            trim_spaces(value_str);

            Token token{type_str, value_str};
            loaded_tokens.push_back(token);
        }

        // 4. close the file, clear existing tokens and add the loaded tokens
        file.close();
        tokens.clear();
        tokens = std::move(loaded_tokens);
        spdlog::info("Successfully loaded {} tokens from file: {}", tokens.size(), filename);
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to load tokens from file {}: {}", filename, e.what());
        throw;
    }
}

const std::vector<Token>& TokenLoader::get_tokens() const {
    return tokens;
}

void TokenLoader::load_from_tokens(const std::vector<Token>& tokens) {
    // Clear existing tokens and load new ones
    this->tokens.clear();
    this->tokens = tokens;
    spdlog::info("Successfully loaded {} tokens from provided token stream.", this->tokens.size());
}