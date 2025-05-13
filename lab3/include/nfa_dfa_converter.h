#ifndef NFA_DFA_CONVERTER_H
#define NFA_DFA_CONVERTER_H

#include "nfa_model.h"
#include "dfa_model.h"

struct NFADFABidirectionalMapping
{
    // Mapping from DFA state(first) to NFA states(second)
    // this is a 1-to-many mapping, one DFA state has many NFA states
    std::unordered_map<std::string, std::unordered_set<std::string>> dfa_to_nfa_mapping;
    // Mapping from NFA state(first) to DFA states(second)
    // this is also a 1-to-many mapping, one NFA state can belong to many closures
    std::unordered_map<std::string, std::unordered_set<std::string>> nfa_to_dfa_mapping;
};

struct NFACTDFAConvertionResult
{
    dfa_model::ConflictTolerantDFA<std::string> dfa; // The converted DFA
    // Mapping from DFA state to NFA states
    NFADFABidirectionalMapping state_mapping;
};

struct NFADFAConvertionResult
{
    dfa_model::DFA<std::string> dfa; // The converted DFA
    // Mapping from DFA state to NFA states
    NFADFABidirectionalMapping state_mapping;
};

class NFA_ConflictToleranceDFA_Converter
{
    public:
        // Constructor
        NFA_ConflictToleranceDFA_Converter() = default;
        virtual ~NFA_ConflictToleranceDFA_Converter() = default;

        // Convert NFA to DFA
        virtual NFACTDFAConvertionResult convert_nfa_to_dfa(const nfa_model::NFA &nfa) = 0;
};

class NFA_DFA_Converter
{
    public:
        // Constructor
        NFA_DFA_Converter() = default;
        virtual ~NFA_DFA_Converter() = default;

        // Convert NFA to DFA
        virtual NFADFAConvertionResult convert_nfa_to_dfa(const nfa_model::NFA &nfa) = 0;
};

#endif // !NFA_DFA_CONVERTER_H