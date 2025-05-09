#ifndef YAML_CFG_LOADER_H
#define YAML_CFG_LOADER_H

#include "cfg_loader.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include <fstream>

class YAML_CFG_Loader : public CFGLoader
{
public:
    // constructor & destructor
    YAML_CFG_Loader();
    ~YAML_CFG_Loader() override;

    cfg_model::CFG LoadCFG(const std::string &filename) override;
};

// helper functions
namespace YAML_CFG_Loader_Helper
{
    // check the validity of the YAML file
    void CheckYAMLFile(const std::string &filename);
    // parse the YAML file and load the CFG
    cfg_model::CFG ParseYAMLFile(const std::string &filename);
    // check the validity of the CFG
    void CheckCFG(const cfg_model::CFG &cfg);
}

#endif // !YAML_CFG_LOADER_H