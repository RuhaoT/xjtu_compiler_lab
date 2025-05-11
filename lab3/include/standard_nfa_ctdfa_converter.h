#ifndef STANDARD_NFA_DFA_CONVERTER_H
#define STANDARD_NFA_DFA_CONVERTER_H

#include "nfa_model.h"
#include "dfa_model.h"
#include "nfa_ctdfa_converter.h"
#include "spdlog/spdlog.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string> 

class StandardNFA_DFA_Converter : public NFA_ConflictToleranceDFA_Converter
{
public:
    // Constructor
    StandardNFA_DFA_Converter();
    ~StandardNFA_DFA_Converter() override;

    // Convert NFA to DFA
    NFACTDFAConvertionResult convert_nfa_to_dfa(const nfa_model::NFA &nfa) override;
};

namespace std_nfa_ctdfa_converter_helper
{
    struct NFAClosure
    {
        std::unordered_set<std::string> states;
        std::string closure_name;
        bool has_accepting_state;
        bool has_initial_state;

        // == operator for NFAClosure
        bool operator==(const NFAClosure &other) const
        {
            // judge by states, accepting_state & initial_state
            return states == other.states && has_initial_state == other.has_initial_state && has_accepting_state == other.has_accepting_state;
        }
    };
}

// hash function for NFAClosure
namespace std
{
    template <>
    struct hash<std_nfa_ctdfa_converter_helper::NFAClosure>
    {
        size_t operator()(const std_nfa_ctdfa_converter_helper::NFAClosure &closure) const
        {
            return std::hash<std::string>()(closure.closure_name);
        }
    };
}

// other helper functions
namespace std_nfa_ctdfa_converter_helper
{
    // get the closure of a state
    NFAClosure get_state_closure(const nfa_model::NFA &nfa, const std::string &state);

    // get the closure of a set of states
    NFAClosure get_state_set_closure(const nfa_model::NFA &nfa, const std::unordered_set<std::string> &states);

    // merge a set of closures
    NFAClosure merge_closures(const std::unordered_set<NFAClosure> &closures);

    // generate the DFA state name
    std::string generate_unique_state_name(const std::unordered_set<std::string> &nfa_state_names);

    // generate the closure set for a given NFA
    std::unordered_set<NFAClosure> generate_closure_set(const nfa_model::NFA &nfa);

    // generate state mapping
    NFADFABidirectionalMapping generate_state_mapping(const std::unordered_set<NFAClosure> &closure_set);

    // generate DFA transitions
    std::unordered_map<std::string, std::multimap<std::string, std::string>> generate_dfa_transitions(const nfa_model::NFA &nfa, const NFADFABidirectionalMapping &state_mapping, dfa_model::ConflictTolerantDFA<std::string> &dfa);
}

#endif // !STANDARD_NFA_DFA_CONVERTER_H