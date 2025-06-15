#include "syntax_semantic_model.h"
#include "spdlog/spdlog.h"

namespace syntax_semantic_model {
// 语义分析用符号表条目，避免与cfg_model::symbol冲突
enum class SymbolEntryType {
    Variable,
    Array,
    Function
};

std::string ProductionInfoMapping::get_node_type(const cfg_model::symbol& lhs, const std::vector<cfg_model::symbol>& rhs) const {
    auto it = production_info.find(lhs);
    if (it != production_info.end()) {
        auto rhs_it = it->second.find(rhs);
        if (rhs_it != it->second.end()) {
            return rhs_it->second;
        } else {
            std::string rhs_str;
            for (const auto& symbol : rhs) {
                rhs_str += std::string(symbol) + " ";
            }
            spdlog::error("No node type found for production {} -> {}", std::string(lhs), rhs_str);
            throw std::runtime_error("No node type found for the given production.");
        }
    } else {
        spdlog::error("No productions found for non-terminal {}", std::string(lhs));
        throw;
    }
}
} // namespace syntax_semantic_model