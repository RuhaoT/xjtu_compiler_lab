#include "flex_based_lexer.h"
#include "reflex/pattern.h"
#include "reflex/matcher.h"
#include "reflex/error.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <sstream>

/*
This backend is pretty much the same as the DFA-based lexer, but it uses regex patterns to match tokens. It calls regex matcher instead of DFA simulator.
*/

FlexBasedLexer::FlexBasedLexer(const FlexLexerSetup &construction_info)
{
    // Initialize token types
    for (const auto &token_type : construction_info.token_types)
    {
        token_types.insert(token_type);
    }

    // Initialize regex patterns
    for (size_t i = 0; i < construction_info.regex_patterns.size(); ++i)
    {
        const auto &token_type = construction_info.token_types[i];
        const auto &regex_pattern = construction_info.regex_patterns[i];

        // Create a regex pattern matcher for each token type
        try
        {
            // !!! convert the regex pattern to unicode
            std::string unicode_pattern = reflex::Matcher::convert(regex_pattern, reflex::convert_flag::unicode);
            regex_patterns[token_type.name] = std::make_unique<reflex::Pattern>(unicode_pattern);
            spdlog::debug("Created regex pattern for token type {}: {}", token_type.name, regex_pattern);
        }
        catch (const reflex::regex_error &e)
        {
            spdlog::error("Failed to create regex pattern for token type {}: {}", token_type.name, e.what());
            throw std::runtime_error("Failed to create regex matcher for token type " + token_type.name + ": " + e.what());
        }
        }
}

LexerResult FlexBasedLexer::Parse(const std::string &input)
{
    // same as DFA-based lexer
    LexerResult result;
    try
    {
        // separate the input string by whitespace
        std::vector<std::string> input_parts;
        std::istringstream iss(input);
        std::string part;
        while (iss >> part)
        {
            input_parts.push_back(part);
        }
        spdlog::debug("Input parts: {}", fmt::join(input_parts, ", "));
        // iterate through each part and parse it to tokens
        std::vector<Token> all_tokens;
        for (const auto &part : input_parts)
        {
            std::vector<Token> tokens = ParseStringToTokens(part);
            all_tokens.insert(all_tokens.end(), tokens.begin(), tokens.end());
        }
        // set the result
        result.success = true;
        result.tokens = all_tokens;
        result.error = "";
    }
    catch (const std::exception &e)
    {
        result.success = false;
        result.error = e.what();
    }
    return result;
}

std::vector<Token> FlexBasedLexer::ParseStringToTokens(const std::string &input)
{
    std::vector<Token> tokens;
    std::string unconsumed_input = input;

    while (!unconsumed_input.empty())
    {
        // Find the longest prefix match for each token type
        FlexBasedLexerHelper::SingleIterationResult result = SingleIterationMatchLongestPrefix(unconsumed_input);

        // Check if a match was found
        if (!result.has_matched)
        {
            spdlog::error("No matching token type found for input: {}", unconsumed_input);
            throw std::runtime_error("No matching token type found for input: " + unconsumed_input);
        }
        else
        {
            // If a match was found, create a token and add it to the list
            Token token;
            token.type = result.longest_prefix_state.token_type_name;
            token.value = result.longest_prefix_state.token_value;

            tokens.push_back(token);
            spdlog::debug("Token found: type = {}, value = {}", token.type, token.value);

            // Update the unconsumed input
            unconsumed_input = unconsumed_input.substr(result.longest_prefix_state.prefix_length);
        }
    }
    return tokens;
}

// check if a give string match a specific token type, this is the only difference from dfa_based_lexer
bool FlexBasedLexer::MatchTokenType(const std::string &input, const TokenType &token_type)
{
    auto it = regex_patterns.find(token_type.name);
    if (it != regex_patterns.end())
    {
        // build the matcher
        reflex::Matcher matcher(*it->second, input);
        // check if the input matches the regex pattern
        if (matcher.matches())
        {
            spdlog::debug("Matched token type: {} with input: {}", token_type.name, input);
            return true;
        }
        else
        {
            spdlog::debug("No match for token type: {} with input: {}", token_type.name, input);
            return false;
        }
    }
    spdlog::error("Token type {} not found in regex pattern matchers", token_type.name);
    return false;
}

// This function is used to find the longest prefix match for a given input string
FlexBasedLexerHelper::SingleIterationResult FlexBasedLexer::SingleIterationMatchLongestPrefix(const std::string &input)
{
    spdlog::debug("Performing single iteration match for input: {}", input);
    FlexBasedLexerHelper::IntermediateState longest_prefix_state;
    bool has_matched = false;

    // iterate through all substrings of the input
    for (size_t length = 1; length <= input.length(); ++length)
    {
        // get the current substring
        std::string current_substring = input.substr(0, length);

        // iterate through all token types
        for (const auto &token_type : token_types)
        {
            // check if the current substring matches the token type
            spdlog::debug("Checking substring: {}, against token type: {}", current_substring, token_type.name);
            if (MatchTokenType(current_substring, token_type))
            {
                spdlog::debug("Matched token type: {}", token_type.name);
                bool update_longest_prefix = false;
                // if the longest prefix state is empty, update it
                if (!has_matched)
                {
                    update_longest_prefix = true;
                    has_matched = true;
                }
                // elif the current substring is longer than the longest prefix state, update it
                else if (length > longest_prefix_state.prefix_length)
                {
                    update_longest_prefix = true;
                }
                // if the current substring is equal to the longest prefix state, perform priority check
                else if (length == longest_prefix_state.prefix_length)
                {
                    if (token_type.priority_level < longest_prefix_state.token_priority_level)
                    {
                        update_longest_prefix = true;
                    }
                    else if (token_type.priority_level == longest_prefix_state.token_priority_level)
                    {
                        // undefined, raise error
                        throw std::runtime_error("Ambiguous token match: " + longest_prefix_state.token_type_name + " and " + token_type.name + " with same priority level: " + std::to_string(token_type.priority_level));
                    }
                }

                // update the longest prefix state
                if (update_longest_prefix)
                {
                    spdlog::debug("Updating longest prefix state: type = {}, value = {}, length = {}, priority = {}", token_type.name, current_substring, length, token_type.priority_level);
                    longest_prefix_state.token_type_name = token_type.name;
                    if (token_type.has_content)
                    {
                        longest_prefix_state.token_value = current_substring;
                    }
                    else
                    {
                        spdlog::debug("Token type {} has no content, setting value to '-'", token_type.name);
                        longest_prefix_state.token_value = "-"; // placeholder for non-content token types
                    }
                    longest_prefix_state.prefix_length = length;
                    longest_prefix_state.token_priority_level = token_type.priority_level;
                }
            }
        }
    }

    // construct the result
    FlexBasedLexerHelper::SingleIterationResult result;
    result.has_matched = has_matched;
    result.longest_prefix_state = longest_prefix_state;
    return result;
};