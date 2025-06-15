#ifndef LEXER_TOKEN_LOADER_H
#define LEXER_TOKEN_LOADER_H

#include "cfg_model.h"
#include "yaml_cfg_loader.h"
#include "yaml-cpp/yaml.h"
#include "syntax_semantic_model.h"
#include "spdlog/spdlog.h"
#include <string>
#include <vector>
#include <fstream>

syntax_semantic_model::ProductionInfoMapping load_semantic_info(const std::string& filename, cfg_model::CFG& target_cfg);

#endif // !LEXER_TOKEN_LOADER_H