#ifndef CONFIG_FRONTEND_H
#define CONFIG_FRONTEND_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "dfa_model.h"

// Config Frontend接口
class ConfigFrontend {
public:
    virtual ~ConfigFrontend() = default;
    
    // 加载配置文件
    virtual bool LoadConfig(const std::string& filepath) = 0;
    
    // 检查DFA配置的合法性
    virtual bool CheckConfig() const = 0;
    
    // 构造DFA数据模型
    virtual DFA ConstructDFA() const = 0;
};

#endif // CONFIG_FRONTEND_H