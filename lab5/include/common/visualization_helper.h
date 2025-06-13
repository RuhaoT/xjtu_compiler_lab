#ifndef VISUALIZATION_HELPER_H
#define VISUALIZATION_HELPER_H

// visualize DFA/NFA/CFG/parsing table, etc.
#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "syntax_semantic_analyzer/symbol_table.h"
#include "syntax_semantic_analyzer/syntax_semantic_model.h"
#include "syntax_semantic_analyzer/ast_model.h"
#include "lr_parsing_model.h"
#include "tabulate/table.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/graph_utility.hpp"
#include "tree/tree.hh"

namespace visualization_helper
{
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> nfa_graph_t;
    typedef boost::graph_traits<nfa_graph_t>::vertex_descriptor vertex_descriptor;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, std::string> dfa_graph_t;
    typedef boost::graph_traits<dfa_graph_t>::vertex_descriptor dfa_vertex_descriptor;
    // prettey print parsing table to console
    void pretty_print_parsing_table(const lr_parsing_model::LRParsingTable &parsing_table, bool export_to_file = false, const std::string &filename = "parsing_table");

    void generate_nfa_dot_file(const nfa_model::NFA &nfa, const std::string &filename, bool generate_svg = false);

    void generate_dfa_dot_file(const dfa_model::DFA<std::string> &dfa, const std::string &filename, bool generate_svg = false);

    void pretty_print_symbol_table(const SymbolTable &symbol_table_manager, bool export_to_file = false, const std::string &filename = "symbol_table");

    void generate_ast_tree_dot_file(
        const tree<std::shared_ptr<ast_model::ASTNodeContent>> &ast_tree,
        const std::string &filename,
        bool generate_svg = false
    );
}
#endif // !VISUALIZATION_HELPER_H