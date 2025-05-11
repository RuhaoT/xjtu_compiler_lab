#ifndef VISUALIZATION_HELPER_H
#define VISUALIZATION_HELPER_H

// visualize DFA/NFA/CFG/parsing table, etc.
#include "cfg_model.h"
#include "nfa_model.h"
#include "dfa_model.h"
#include "lr_parsing_model.h"
#include "tabulate/table.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/graph_utility.hpp"

namespace visualization_helper
{
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> nfa_graph_t;
    typedef boost::graph_traits<nfa_graph_t>::vertex_descriptor vertex_descriptor;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, std::string> dfa_graph_t;
    typedef boost::graph_traits<dfa_graph_t>::vertex_descriptor dfa_vertex_descriptor;
    // prettey print parsing table to console
    void pretty_print_parsing_table(const lr_parsing_model::LRParsingTable &parsing_table);

    void generate_nfa_dot_file(const nfa_model::NFA &nfa, const std::string &filename, bool generate_svg = false);

    void generate_ctdfa_dot_file(const dfa_model::ConflictTolerantDFA<std::string> &dfa, const std::string &filename, bool generate_svg = false);
}
#endif // !VISUALIZATION_HELPER_H