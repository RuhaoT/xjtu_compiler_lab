#ifndef YAML_CONFIG_FRONTEND_H
#define YAML_CONFIG_FRONTEND_H

#include "config_frontend.h"
#include <yaml-cpp/yaml.h>
#include <memory>

class YAMLConfigFrontend : public ConfigFrontend {
private:
    std::unique_ptr<YAML::Node> config;
    dfa_model::DFA<char> dfa;
    bool is_loaded;
    bool is_checked;

public:
    YAMLConfigFrontend();
    ~YAMLConfigFrontend() override;

    // 实现ConfigFrontend接口方法
    bool LoadConfig(const std::string& filepath) override;
    bool CheckConfig() const override;
    dfa_model::DFA<char> ConstructDFA() const override;

private:
    // 辅助方法
    bool ParseCharacterSet(const YAML::Node& node);
    bool ParseStatesSet(const YAML::Node& node);
    bool ParseInitialState(const YAML::Node& node);
    bool ParseAcceptingStates(const YAML::Node& node);
    bool ParseTransitions(const YAML::Node& node);
};

#endif // YAML_CONFIG_FRONTEND_H