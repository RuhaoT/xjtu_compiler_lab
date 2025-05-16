#ifndef DFA_SIMULATOR_H
#define DFA_SIMULATOR_H

#include <string>
#include <set>
#include "dfa_model.h"

// more versatile DFA simulator with template
template<typename T>
class DFASimulator {
public:
    virtual ~DFASimulator() = default;

    // update dfa setting
    virtual bool UpdateDFA(const dfa_model::DFA<T>& dfa) = 0;

    // simulate an array of characters with type T
    virtual bool SimulateString(const std::vector<T>& input) = 0;
};


#endif // !DFA_SIMULATOR_H