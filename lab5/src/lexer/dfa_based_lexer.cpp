#include "dfa_based_lexer.h"
#include "standard_dfa_simulator.h"
#include "spdlog/spdlog.h"
#include <exception>
#include <iostream>
#include <sstream>

DFABasedLexer::DFABasedLexer(const DFALexerSetup &construction_info)
{
    // assume: 1. the DFA configurations number is equal to the token types number
    // 2. each token type corresponds to the same index of the DFA configurations
    // 3. both token types and DFA configurations are valid
    int num_types = construction_info.token_types.size();
    // start iterating from 0
    for (int i = 0; i < num_types; ++i)
    {
        // add the token type to the set
        token_types.insert(construction_info.token_types[i]);

        // create a new DFA simulator for each token type
        auto dfa_simulator = std::make_unique<StandardDFASimulator>();

        // update the DFA simulator with the corresponding DFA configuration
        if (!dfa_simulator->UpdateDFA(construction_info.dfa_configurations[i]))
        {
            throw std::runtime_error("Failed to update DFA simulator with the given DFA configuration");
        }
        // add the DFA simulator to the map
        dfa_simulators[construction_info.token_types[i].name] = std::move(dfa_simulator);

        // check if the DFA simulator was created successfully
        if (dfa_simulators[construction_info.token_types[i].name] == nullptr)
        {
            throw std::runtime_error("Failed to create DFA simulator for token type: " + construction_info.token_types[i].name);
        }
    }
}

DFABasedLexer::~DFABasedLexer() = default;

bool DFABasedLexer::MatchTokenType(const std::string &input, const TokenType &token_type) const
{
    // TODO: log handling
    std::string simulation_log;

    auto it = dfa_simulators.find(token_type.name);
    if (it != dfa_simulators.end())
    {
        std::vector<char> character_vector(input.begin(), input.end());
        return it->second->SimulateString(character_vector);
    }
    return false;
}

DFABasedLexerHelper::SingleIterationResult DFABasedLexer::SingleIterationMatchLongestPrefix(const std::string &input) const
{
    spdlog::debug("Performing single iteration match for input: {}", input);
    DFABasedLexerHelper::IntermediateState longest_prefix_state;
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
    DFABasedLexerHelper::SingleIterationResult result;
    result.has_matched = has_matched;
    result.longest_prefix_state = longest_prefix_state;
    return result;
};

std::vector<Token> DFABasedLexer::ParseStringToTokens(const std::string &input) const
{
    std::vector<Token> tokens;
    std::string unconsumed_input = input;

    // Iterate until the input is fully consumed
    while (!unconsumed_input.empty())
    {
        // try to match the longest prefix with current prefix length
        DFABasedLexerHelper::SingleIterationResult result = SingleIterationMatchLongestPrefix(unconsumed_input);

        // check if a match was found
        if (!result.has_matched)
        {
            throw std::runtime_error("No matching token type found for input: " + unconsumed_input);
        }
        else
        {
            // if a match was found, create a token and add it to the list
            Token token;
            token.type = result.longest_prefix_state.token_type_name;
            token.value = result.longest_prefix_state.token_value;

            tokens.push_back(token);
            spdlog::debug("Token found: type = {}, value = {}", token.type, token.value);

            // update the unconsumed input
            unconsumed_input = unconsumed_input.substr(result.longest_prefix_state.prefix_length);
        }
    }
    return tokens;
}

LexerResult DFABasedLexer::Parse(const std::string &input)
{
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