#ifndef CFG_ANALYZER_H
#define CFG_ANALYZER_H

#include "cfg_model.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

class CFGAnalyzer
{
private:
    cfg_model::CFG cfg;
    cfg_model::FirstSet first_set;
    cfg_model::FollowSet follow_set;

public:
    CFGAnalyzer(const cfg_model::CFG &cfg);
    ~CFGAnalyzer();

    // Compute the first set for the CFG
    void computeFirstSet();

    // Compute the follow set for the CFG
    void computeFollowSet();

    // Get the first set
    const cfg_model::FirstSet &getFirstSet() const;

    // Get the follow set
    const cfg_model::FollowSet &getFollowSet() const;
};

#endif // !CFG_ANALYZER_H