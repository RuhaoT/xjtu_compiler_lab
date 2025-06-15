#ifndef CFG_LOADER_H
#define CFG_LOADER_H

#include "cfg_model.h"

class CFGLoader
{
public:

    virtual ~CFGLoader() = default;
    // Load a CFG from a file
    virtual cfg_model::CFG LoadCFG(const std::string &filename) = 0;
};

#endif // !CFG_LOADER_H