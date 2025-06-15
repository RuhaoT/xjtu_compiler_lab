#include "visualization_helper.h"
#include "spdlog/spdlog.h"
#include "tabulate/table.hpp"
#include "tabulate/markdown_exporter.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/graph_utility.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <map> // For std::map (used in property maps)
#include <cstdlib> // For std::system (if using generate_svg)
#include <boost/property_map/dynamic_property_map.hpp> // For boost::dynamic_properties
#include "syntax_semantic_analyzer/symbol_table.h" // Ensure this path is correct
#include "syntax_semantic_analyzer/ast_model.h"   // Ensure this path is correct
#include "syntax_semantic_analyzer/syntax_semantic_model.h" // Ensure this path is correct
#include "tree/tree.hh"

void visualization_helper::pretty_print_parsing_table(const lr_parsing_model::LRParsingTable &parsing_table, bool export_to_file, const std::string &filename)
{
    try{
    // Create a table using the tabulate library
    tabulate::Table table_parsing;
    tabulate::Table table_state_index;
    tabulate::Table table_production_index;

    int symbol_count = parsing_table.all_symbols.size();
    int state_count = parsing_table.all_states.size();

    // map each state to its index
    std::unordered_map<std::string, int> state_index_map;
    int index = 0;
    for (const auto &state : parsing_table.all_states)
    {
        spdlog::debug("State {}: \n {}", std::to_string(index), state);
        state_index_map[state] = index++;
    }

    // map each production rule to its index when meet
    std::unordered_map<lr_parsing_model::Action, int> production_index_map;
    int production_index = 0;

    // table_parsing
    // copy the symbol set
    std::vector<cfg_model::symbol> symbol_set(parsing_table.all_symbols.begin(), parsing_table.all_symbols.end());
    std::sort(symbol_set.begin(), symbol_set.end(), [](const cfg_model::symbol &a, const cfg_model::symbol &b) {
        return (a.is_terminal) && (!b.is_terminal);
    });
    // header row: state names, followed by all symbols
    tabulate::Table::Row_t header_row;
    header_row.push_back("State Index");
    for (const auto &symbol : symbol_set)
    {
        header_row.push_back(symbol.name);
    }
    table_parsing.add_row(header_row);
    // fill the table with action and goto information

    for (const auto &state : parsing_table.all_states)
    {
        tabulate::Table::Row_t row;
        std::vector<int> conflict_cells;
        row.push_back(std::to_string(state_index_map[state]));
        spdlog::debug("Current row state index: {}", state_index_map[state]);
        // start building each cell
        for (const auto &symbol : symbol_set)
        {
            // check if the cell is empty
            if (parsing_table.check_cell_empty(state, symbol))
            {
                std::string error_message = "Error: Parsing table cell is empty for state " + state + " and symbol " + symbol.name;
                spdlog::error(error_message);
                throw std::runtime_error(error_message);
            }
            // get the actions and gotos for the current state and symbol
            // check if the symbol is a terminal or non-terminal
            bool has_action_conflict = false;
            bool has_goto_conflict = false;
            if (symbol.is_terminal)
            {
            auto actions = parsing_table.get_actions(state, symbol);
            has_action_conflict = actions.size() > 1;
            std::string cell_content = "";
            for (const auto &action: actions)
            {
                if (action.action_type == "shift")
                {
                    cell_content += "S" + std::to_string(state_index_map[action.target_state]) + "/";
                }
                else if (action.action_type == "reduce")
                {
                    // check if the production rule is already in the map
                    if (production_index_map.find(action) == production_index_map.end())
                    {
                        // add the production rule to the map
                        production_index_map[action] = production_index;
                        production_index++;
                    }
                    cell_content += "R" + std::to_string(production_index_map[action]) + "/";
                }
                else if (action.action_type == "accept")
                {
                    cell_content += "ACC/";
                }
                else if (action.action_type == "empty")
                {
                    ;
                }
                else
                {
                    std::string error_message = "Error: Unknown action type " + action.action_type;
                    spdlog::error(error_message);
                    throw std::runtime_error(error_message);
                }
            }
            // remove the last '/' if it exists
            if (!cell_content.empty() && cell_content.back() == '/')
            {
                cell_content.pop_back();
            }
            // add the cell content to the row
            row.push_back(cell_content);
            // if has conflict, mark the conflict cell
            if (has_action_conflict)
            {
                conflict_cells.push_back(row.size() - 1);
            }
        }
        else
        {
            // if the symbol is a non-terminal, get the gotos
            auto gotos = parsing_table.get_gotos(state, symbol);
            has_goto_conflict = gotos.size() > 1;
            std::string cell_content = "";
            for (const auto &goto_state : gotos)
            {
                if (!(goto_state == ""))
                {
                cell_content += std::to_string(state_index_map[goto_state]) + "/";
                }
            }
            // remove the last '/' if it exists
            if (!cell_content.empty() && cell_content.back() == '/')
            {
                cell_content.pop_back();
            }
            // add the cell content to the row
            row.push_back(cell_content);
            // if has conflict, mark the conflict cell
            if (has_goto_conflict)
            {
                conflict_cells.push_back(row.size() - 1);
            }
        }
        }
        // add the row to the table
        table_parsing.add_row(row);
        // set the cell color to red if has conflict
        for (const auto &cell_index : conflict_cells)
        {
            table_parsing[state_index_map[state] + 1][cell_index].format()
                .font_color(tabulate::Color::red)
                .font_style({tabulate::FontStyle::bold});
        }
    }
    spdlog::debug("Parsing table visualization completed with {} rows and {} columns", state_count, symbol_count + 1);

    // table_state_index
    // header row: state index, state name
    tabulate::Table::Row_t header_row_state_index = {"State Index", "State Name"};
    table_state_index.add_row(header_row_state_index);
    // fill the table with state index and state name
    for (const auto &state : parsing_table.all_states)
    {
        tabulate::Table::Row_t row;
        row.push_back(std::to_string(state_index_map[state]));
        row.push_back(state);
        table_state_index.add_row(row);
    }
    spdlog::debug("State index table visualization completed with {} rows", state_count);

    // table_production_index
    // header row: production index, production rule
    tabulate::Table::Row_t header_row_production_index = {"Production Index", "Production Rule"};
    table_production_index.add_row(header_row_production_index);
    // fill the table with production index and production rule
    for (const auto &pair : production_index_map)
    {
        tabulate::Table::Row_t row;
        row.push_back(std::to_string(pair.second));
        std::string production_rule = pair.first.reduce_rule_lhs.name + " -> ";
        for (const auto &symbol : pair.first.reduce_rule_rhs)
        {
            production_rule += symbol.name + " ";
        }
        row.push_back(production_rule);
        table_production_index.add_row(row);
    }
    spdlog::debug("Production index table visualization completed with {} rows", production_index_map.size());

    if (!export_to_file)
    {
    // print the tables
    std::cout << "Parsing Table:" << std::endl;
    std::cout << table_parsing << std::endl;
    std::cout << "State Index Table:" << std::endl;
    std::cout << table_state_index << std::endl;
    std::cout << "Production Index Table:" << std::endl;
    std::cout << table_production_index << std::endl;

    // print the start state
    std::cout << "Start State: " << parsing_table.start_state << std::endl;
    }
    else
    {
        tabulate::MarkdownExporter markdown_exporter;
        // Export the tables to markdown format
        auto table_parsing_markdown = markdown_exporter.dump(table_parsing);
        auto table_state_index_markdown = markdown_exporter.dump(table_state_index);
        auto table_production_index_markdown = markdown_exporter.dump(table_production_index);
        // Write the markdown to files
        // if already exists, overwrite it
        std::ofstream parsing_table_file(filename);
        if (!parsing_table_file.is_open())
        {
            spdlog::error("Failed to open file for writing: {}", filename);
            throw std::runtime_error("Failed to open file: " + filename);
        }
        parsing_table_file << "# Parsing Table\n" << table_parsing_markdown << "\n\n";
        parsing_table_file << "# State Index Table\n" << table_state_index_markdown << "\n\n";
        parsing_table_file << "# Production Index Table\n" << table_production_index_markdown << "\n\n";
        parsing_table_file << "Start State: " << parsing_table.start_state << "\n";
        parsing_table_file.close();
        spdlog::info("Parsing table visualization exported to file: {}", filename);
    }

}
    catch (const std::exception &e)
    {
        std::string error_message = "Error visualizing parsing table: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

void visualization_helper::generate_nfa_dot_file(const nfa_model::NFA &nfa, const std::string &filename, bool generate_svg)
{
    try
    {
        nfa_graph_t g;
        std::ofstream dot_file(filename);
        if (!dot_file.is_open())
        {
            spdlog::error("Failed to open .dot file for writing: {}", filename);
            throw std::runtime_error("Failed to open .dot file: " + filename);
        }

        std::unordered_map<std::string, vertex_descriptor> state_to_vertex_map;

        // Add vertices for each NFA state
        for (const auto &state_name : nfa.states)
        {
            vertex_descriptor v = boost::add_vertex(g);
            state_to_vertex_map[state_name] = v;
        }

        // Property maps for Graphviz styling
        std::map<vertex_descriptor, std::string> gv_vertex_ids;      // Node IDs (names) in Graphviz
        std::map<vertex_descriptor, std::string> gv_vertex_shapes;   // Node shapes
        std::map<boost::graph_traits<nfa_graph_t>::edge_descriptor, std::string> gv_edge_labels; // Edge labels

        // Populate vertex properties
        for (const auto &pair : state_to_vertex_map)
        {
            const std::string &state_name = pair.first;
            vertex_descriptor v = pair.second;

            gv_vertex_ids[v] = state_name; // Use NFA state name as Graphviz node ID

            if (nfa.accepting_states.count(state_name))
            {
                gv_vertex_shapes[v] = "doubleoctagon";
            }
            else
            {
                gv_vertex_shapes[v] = "rect";
            }
        }

        // Special handling for the start state visualization
        if (!nfa.start_state.empty())
        {
            if (state_to_vertex_map.count(nfa.start_state))
            {
                vertex_descriptor inv_start_node = boost::add_vertex(g);
                // Assign a unique ID for the invisible node, ensuring it doesn't clash with actual state names
                std::string inv_node_id = "__invisible_start_node__";
                int counter = 0;
                while(nfa.states.count(inv_node_id)) { // Ensure uniqueness if a state is named like this
                    inv_node_id = "__invisible_start_node__" + std::to_string(counter++);
                }
                gv_vertex_ids[inv_start_node] = inv_node_id;
                gv_vertex_shapes[inv_start_node] = "point"; // Makes the node small and inconspicuous

                vertex_descriptor actual_start_vertex = state_to_vertex_map[nfa.start_state];
                boost::add_edge(inv_start_node, actual_start_vertex, g); // Edge from invisible to actual start
                // No label for this edge, so it's not added to gv_edge_labels
            }
            else
            {
                spdlog::warn("NFA start state '{}' defined but not found in the set of NFA states. Start arrow will not be drawn.", nfa.start_state);
            }
        }

        // Add non-epsilon transitions
        for (const auto &source_pair : nfa.non_epsilon_transitions)
        {
            const std::string &from_state = source_pair.first;
            if (state_to_vertex_map.find(from_state) == state_to_vertex_map.end())
            {
                spdlog::warn("Source state '{}' in non-epsilon transition not found in NFA states. Skipping transitions from it.", from_state);
                continue;
            }
            vertex_descriptor u = state_to_vertex_map[from_state];

            for (const auto &target_pair : source_pair.second)
            {
                const std::string &symbol = target_pair.first;
                const std::string &to_state = target_pair.second;
                if (state_to_vertex_map.find(to_state) == state_to_vertex_map.end())
                {
                    spdlog::warn("Target state '{}' for symbol '{}' from state '{}' not found in NFA states. Skipping this transition.", to_state, symbol, from_state);
                    continue;
                }
                vertex_descriptor v = state_to_vertex_map[to_state];
                auto edge = boost::add_edge(u, v, g);
                gv_edge_labels[edge.first] = symbol;
            }
        }

        // Add epsilon transitions
        for (const auto &epsilon_trans : nfa.epsilon_transitions)
        {
            const std::string &from_state = epsilon_trans.first;
            const std::string &to_state = epsilon_trans.second;

            if (state_to_vertex_map.find(from_state) == state_to_vertex_map.end())
            {
                spdlog::warn("Source state '{}' in epsilon transition not found in NFA states. Skipping.", from_state);
                continue;
            }
            if (state_to_vertex_map.find(to_state) == state_to_vertex_map.end())
            {
                spdlog::warn("Target state '{}' in epsilon transition not found in NFA states. Skipping.", to_state);
                continue;
            }

            vertex_descriptor u = state_to_vertex_map[from_state];
            vertex_descriptor v = state_to_vertex_map[to_state];
            auto edge = boost::add_edge(u, v, g);
            gv_edge_labels[edge.first] = "ε";
        }

        boost::dynamic_properties dp;
        dp.property("node_id", boost::make_assoc_property_map(gv_vertex_ids));
        dp.property("shape", boost::make_assoc_property_map(gv_vertex_shapes));
        dp.property("label", boost::make_assoc_property_map(gv_edge_labels));
        // Graph attributes
        dp.property("rankdir", boost::make_constant_property<nfa_graph_t *>(std::string("LR"))); // Left-to-Right layout
        dp.property("splines", boost::make_constant_property<nfa_graph_t *>(std::string("true")));

        boost::write_graphviz_dp(dot_file, g, dp);
        dot_file.close();

        spdlog::info("NFA graph written to {}", filename);

        if (generate_svg)
        {
            // Ensure filename is properly quoted if it might contain spaces, though not strictly necessary for typical filenames.
            std::string command = "dot -Tsvg \"" + filename + "\" -o \"" + filename + ".svg\"";
            spdlog::info("Attempting to generate SVG: {}", command);
            int result = std::system(command.c_str());
            if (result == 0)
            {
                spdlog::info("SVG file generated: {}.svg", filename);
            }
            else
            {
                spdlog::error("Failed to generate SVG with command: '{}'. 'dot' utility might not be installed or not in PATH. Error code: {}", command, result);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating NFA .dot file: ";
        error_message += e.what();
        spdlog::error(error_message);
    }
}

// generate dfa dot file, this is basically the same as nfa dot file, but without considering epsilon transitions
void visualization_helper::generate_dfa_dot_file(const dfa_model::DFA<std::string> &dfa, const std::string &filename, bool generate_svg)
{
    try
    {
        dfa_graph_t g;
        std::ofstream dot_file(filename);
        if (!dot_file.is_open())
        {
            spdlog::error("Failed to open .dot file for writing: {}", filename);
            throw std::runtime_error("Failed to open .dot file: " + filename);
        }

        std::unordered_map<std::string, dfa_vertex_descriptor> state_to_vertex_map;

        // Add vertices for each DFA state
        for (const auto &state_name : dfa.states_set)
        {
            dfa_vertex_descriptor v = boost::add_vertex(g);
            state_to_vertex_map[state_name] = v;
        }

        // Property maps for Graphviz styling
        std::map<dfa_vertex_descriptor, std::string> gv_vertex_ids;      // Node IDs (names) in Graphviz
        std::map<dfa_vertex_descriptor, std::string> gv_vertex_shapes;   // Node shapes
        std::map<boost::graph_traits<dfa_graph_t>::edge_descriptor, std::string> gv_edge_labels; // Edge labels

        // Populate vertex properties
        for (const auto &pair : state_to_vertex_map)
        {
            const std::string &state_name = pair.first;
            dfa_vertex_descriptor v = pair.second;

            gv_vertex_ids[v] = state_name; // Use DFA state name as Graphviz node ID

            if (dfa.accepting_states.count(state_name))
            {
                gv_vertex_shapes[v] = "doubleoctagon";
            }
            else
            {
                gv_vertex_shapes[v] = "rect";
            }
        }

        // Special handling for the start state visualization
        if (!dfa.initial_state.empty())
        {
            if (state_to_vertex_map.count(dfa.initial_state))
            {
                dfa_vertex_descriptor inv_start_node = boost::add_vertex(g);
                // Assign a unique ID for the invisible node, ensuring it doesn't clash with actual state names
                std::string inv_node_id = "__invisible_start_node__";
                int counter = 0;
                while(dfa.states_set.count(inv_node_id)) { // Ensure uniqueness if a state is named like this
                    inv_node_id = "__invisible_start_node__" + std::to_string(counter++);
                }
                gv_vertex_ids[inv_start_node] = inv_node_id;
                gv_vertex_shapes[inv_start_node] = "point"; // Makes the node small and inconspicuous
                dfa_vertex_descriptor actual_start_vertex = state_to_vertex_map[dfa.initial_state];
                boost::add_edge(inv_start_node, actual_start_vertex, g); // Edge from invisible to actual start
                // No label for this edge, so it's not added to gv_edge_labels
            }
            else
            {
                spdlog::warn("DFA start state '{}' defined but not found in the set of DFA states. Start arrow will not be drawn.", dfa.initial_state);
            }
        }
        // Add transitions
        for (const auto &source_pair : dfa.transitions)
        {
            const std::string &from_state = source_pair.first;
            if (state_to_vertex_map.find(from_state) == state_to_vertex_map.end())
            {
                spdlog::warn("Source state '{}' in transition not found in DFA states. Skipping transitions from it.", from_state);
                continue;
            }
            dfa_vertex_descriptor u = state_to_vertex_map[from_state];

            for (const auto &target_pair : source_pair.second)
            {
                const std::string &symbol = target_pair.first;
                const std::string &to_state = target_pair.second;
                if (state_to_vertex_map.find(to_state) == state_to_vertex_map.end())
                {
                    spdlog::warn("Target state '{}' for symbol '{}' from state '{}' not found in DFA states. Skipping this transition.", to_state, symbol, from_state);
                    continue;
                }
                dfa_vertex_descriptor v = state_to_vertex_map[to_state];
                auto edge = boost::add_edge(u, v, g);
                gv_edge_labels[edge.first] = symbol;
            }
        }
        boost::dynamic_properties dp;
        dp.property("node_id", boost::make_assoc_property_map(gv_vertex_ids));
        dp.property("shape", boost::make_assoc_property_map(gv_vertex_shapes));
        dp.property("label", boost::make_assoc_property_map(gv_edge_labels));
        // Graph attributes
        dp.property("rankdir", boost::make_constant_property<dfa_graph_t *>(std::string("LR"))); // Left-to-Right layout
        dp.property("splines", boost::make_constant_property<dfa_graph_t *>(std::string("true")));
        boost::write_graphviz_dp(dot_file, g, dp);
        dot_file.close();
        spdlog::info("DFA graph written to {}", filename);
        if (generate_svg)
        {
            // Ensure filename is properly quoted if it might contain spaces, though not strictly necessary for typical filenames.
            std::string command = "dot -Tsvg \"" + filename + "\" -o \"" + filename + ".svg\"";
            spdlog::info("Attempting to generate SVG: {}", command);
            int result = std::system(command.c_str());
            if (result == 0)
            {
                spdlog::info("SVG file generated: {}.svg", filename);
            }
            else
            {
                spdlog::error("Failed to generate SVG with command: '{}'. 'dot' utility might not be installed or not in PATH. Error code: {}", command, result);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string error_message = "Error generating DFA .dot file: ";
        error_message += e.what();
        spdlog::error(error_message);
    }
};

void visualization_helper::pretty_print_symbol_table(const SymbolTable &symbol_table_manager, bool export_to_file, const std::string &filename) {
    try {
        tabulate::Table symbol_table_viz;
        symbol_table_viz.add_row({"Symbol Name", "Data Type", "Kind", "Scope ID", "Memory Size (bytes)", "Other Attributes"});

        for (const syntax_semantic_model::SymbolEntry &entry : symbol_table_manager.symbols) {
            tabulate::Table::Row_t row_data;
            row_data.push_back(entry.symbol_name);
            row_data.push_back(entry.data_type);

            std::string kind_str;
            std::stringstream other_attrs_ss;

            switch (entry.symbol_type) {
                case syntax_semantic_model::SymbolType::Variable:
                    kind_str = "Variable";
                    break;
                case syntax_semantic_model::SymbolType::Function:
                    kind_str = "Function";
                    other_attrs_ss << "Args: [";
                    if (entry.arg_list.has_value()) {
                        for (size_t i = 0; i < entry.arg_list->size(); ++i) {
                            other_attrs_ss << entry.arg_list.value()[i];
                            if (i < entry.arg_list->size() - 1) other_attrs_ss << ", ";
                        }
                    }
                    other_attrs_ss << "]";
                    if (entry.direct_child_scope.has_value()) {
                        other_attrs_ss << ", BodyScopeID: " << entry.direct_child_scope.value();
                    } else {
                        other_attrs_ss << ", BodyScopeID: N/A";
                    }
                    break;
                case syntax_semantic_model::SymbolType::Array:
                    kind_str = "Array";
                    if (entry.array_length.has_value()) {
                        other_attrs_ss << "Length: " << entry.array_length.value();
                    } else {
                        other_attrs_ss << "Length: N/A";
                    }
                    break;
                default:
                    kind_str = "Unknown";
                    break;
            }

            row_data.push_back(kind_str);
            row_data.push_back(std::to_string(entry.scope_id));
            row_data.push_back(std::to_string(entry.memory_size));
            row_data.push_back(other_attrs_ss.str());

            symbol_table_viz.add_row(row_data);
        }

        if (!export_to_file) {
            std::cout << "\nSymbol Table:" << std::endl;
            std::cout << symbol_table_viz << std::endl;
            spdlog::debug("Symbol table visualization completed.");
        } else {
            tabulate::MarkdownExporter markdown_exporter;
            auto table_markdown = markdown_exporter.dump(symbol_table_viz);
            std::ofstream out_file(filename);
            if (!out_file.is_open()) {
                spdlog::error("Failed to open file for writing: {}", filename);
                throw std::runtime_error("Failed to open file: " + filename);
            }
            out_file << "# Symbol Table\n" << table_markdown << "\n";
            out_file.close();
            spdlog::info("Symbol table exported to file: {}", filename);
        }
    } catch (const std::exception &e) {
        std::string error_message = "Error visualizing symbol table: ";
        error_message += e.what();
        spdlog::error(error_message);
    }
}

void visualization_helper::generate_ast_tree_dot_file(
    const tree<std::shared_ptr<ast_model::ASTNodeContent>> &ast_tree,
    const std::string &filename,
    bool generate_svg
) {
    try {
        std::ofstream dot_file(filename);
        if (!dot_file.is_open()) {
            spdlog::error("Failed to open .dot file for writing: {}", filename);
            throw std::runtime_error("Failed to open .dot file: " + filename);
        }

        // 定义 graph 类型
        using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
        Graph g;

        // 记录每个 AST 节点在 graph 中的 vertex_descriptor
        std::unordered_map<std::shared_ptr<ast_model::ASTNodeContent>, Graph::vertex_descriptor> node_to_vertex;
        // 记录每个 vertex 的 label 和 node_id
        std::unordered_map<Graph::vertex_descriptor, std::string> vertex_labels;
        std::unordered_map<Graph::vertex_descriptor, std::string> vertex_ids;

        int vid = 0;
        for (auto it = ast_tree.begin_breadth_first(); it != ast_tree.end_breadth_first(); ++it) {
            spdlog::debug("Processing AST node: {}", it.node->data ? ast_model::ast_node_type_to_string(it.node->data->node_type) : "(null)");
            auto v = boost::add_vertex(g);
            node_to_vertex[it.node->data] = v;
            
            // label
            if (it.node->data) { // it.node->data is std::shared_ptr<ast_model::ASTNodeContent>
                // 假设 ast_model::ASTNodeContent 有一个 virtual std::string to_string() const; 方法
                vertex_labels[v] = it.node->data->to_string();
            } else {
                vertex_labels[v] = "(null)";
            }
            
            // node_id 必须唯一且为字符串
            vertex_ids[v] = "n" + std::to_string(vid++);
            spdlog::debug("Adding vertex: {} with label: {}", vertex_ids[v], vertex_labels[v]);
        }

        // 2. 添加所有边
        for (auto it = ast_tree.begin_breadth_first(); it != ast_tree.end_breadth_first(); ++it) {
            if (!it.node->data) continue; // Skip if data is null
            auto parent_v_it = node_to_vertex.find(it.node->data);
            if (parent_v_it == node_to_vertex.end()) {
                spdlog::warn("Parent node not found in vertex map during edge creation.");
                continue;
            }
            auto parent_v = parent_v_it->second;
            
            int children_count = it.number_of_children();
            for (int i = 0; i < children_count; ++i) {
                auto child_it = ast_tree.child(it, i);
                if (!child_it.node->data) continue; // Skip if child data is null

                auto child_v_it = node_to_vertex.find(child_it.node->data);
                if (child_v_it == node_to_vertex.end()) {
                     spdlog::warn("Child node not found in vertex map during edge creation.");
                     continue;
                }
                auto child_v = child_v_it->second;
                boost::add_edge(parent_v, child_v, g);
            }
        }

        // 3. 绑定 label 和 node_id 属性
        boost::dynamic_properties dp;
        dp.property("label", boost::make_assoc_property_map(vertex_labels));
        dp.property("node_id", boost::make_assoc_property_map(vertex_ids));

        // 4. 写入 dot 文件
        boost::write_graphviz_dp(dot_file, g, dp);
        dot_file.close();
        spdlog::info("AST tree written to {}", filename);

        // 5. 可选生成 svg
        if (generate_svg) {
            std::string command = "dot -Tsvg \"" + filename + "\" -o \"" + filename + ".svg\"";
            spdlog::info("Attempting to generate SVG: {}", command);
            int result = std::system(command.c_str());
            if (result == 0) {
                spdlog::info("SVG file generated: {}.svg", filename);
            } else {
                spdlog::error("Failed to generate SVG with command: '{}'. 'dot' utility might not be installed or not in PATH. Error code: {}", command, result);
            }
        }
    }
    catch (const std::exception &e) {
        std::string error_message = "Error generating AST tree .dot file: ";
        error_message += e.what();
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
    }