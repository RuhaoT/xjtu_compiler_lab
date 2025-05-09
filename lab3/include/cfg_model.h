#ifndef CFG_MODEL_H
#define CFG_MODEL_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional> // Required for std::hash

namespace cfg_model
{
    struct symbol
    {
        std::string name;
        bool is_terminal;

        // overload the equality operator
        bool operator==(const symbol &other) const
        {
            return name == other.name && is_terminal == other.is_terminal;
        }

        // string representation
        operator std::string() const
        {
            return is_terminal ? name + "_T" : name + "_NT";
        }
    };
} // namespace cfg_model

// Specialization of std::hash for cfg_model::symbol
// This must be defined after cfg_model::symbol and before its first use.
namespace std
{
    template <>
    struct hash<cfg_model::symbol>
    {
        size_t operator()(const cfg_model::symbol &s) const
        {
            // Use the same logic as your symbol_hash or a refined one
            size_t h1 = hash<string>()(s.name);
            size_t h2 = hash<bool>()(s.is_terminal);
            return h1 ^ h2; // Or boost::hash_combine
        }
    };
} // namespace std

// Specialization of std::hash for std::vector<cfg_model::symbol>
namespace std
{
    template <>
    struct hash<std::vector<cfg_model::symbol>>
    {
        size_t operator()(const std::vector<cfg_model::symbol> &vec) const
        {
            size_t seed = vec.size();
            for (const auto &s : vec)
            {
                // Combine hashes of individual symbols
                // Using a common way to combine hashes (boost::hash_combine logic)
                seed ^= hash<cfg_model::symbol>()(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
} // namespace std

namespace cfg_model {
    struct CFG
    {
        cfg_model::symbol start_symbol;
        std::unordered_set<cfg_model::symbol> terminals;
        std::unordered_set<cfg_model::symbol> non_terminals;
        std::unordered_map<cfg_model::symbol, std::unordered_set<std::vector<cfg_model::symbol>>> production_rules;
        std::unordered_set<cfg_model::symbol> epsilon_production_symbols;
    };
} // namespace cfg_model

#endif // !CFG_MODEL_H