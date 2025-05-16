#ifndef NFA_MODEL_H
#define NFA_MODEL_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>

namespace nfa_model {

    struct NFA {
        std::unordered_set<std::string> states;
        std::unordered_set<std::string> character_set;
        std::string start_state;
        std::unordered_set<std::string> accepting_states;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> non_epsilon_transitions;
        std::multimap<std::string, std::string>epsilon_transitions;

        int count_non_epsilon_transitions() const {
            int count = 0;
            for (const auto& ne_transition : non_epsilon_transitions) {
                count += ne_transition.second.size();
            }
            return count;
        }
    };
};

namespace nfa_model_helper {
    // helper function to check if the NFA is valid
    bool check_nfa_configuration(const nfa_model::NFA& nfa);
}
#endif // !NFA_MODEL_H