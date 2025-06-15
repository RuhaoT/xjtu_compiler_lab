#include "syntax_semantic_analyzer/interm_code_generator.h"
#include "tree/tree.hh"
#include "ast_model.h"
#include "interm_code/interm_code_model.h"
#include "logical_env_simulator.h"
#include "scope_table.h"
#include "spdlog/spdlog.h"
#include "symbol_table.h"
#include <fstream>
#include <stdexcept>
#include <iomanip> // Required for std::setw

IntermCodeGenerator::IntermCodeGenerator(
    const SymbolTable& symbol_table,
    tree<std::shared_ptr<ast_model::ASTNodeContent>> ast_tree
) : logical_env_simulator(symbol_table), current_ast_tree(ast_tree) {
    
    spdlog::info("IntermCodeGenerator initialized with symbol table and AST tree.");
}

void IntermCodeGenerator::produce_intermediate_code(const std::string& output_file) {
    // 1. init the logical environment simulator
    logical_env_simulator.init();
    spdlog::info("Starting intermediate code generation...");
    // 2. check if the AST tree is empty
    if (current_ast_tree.empty()) {
        spdlog::error("AST tree is empty. Cannot generate intermediate code.");
        throw std::runtime_error("AST tree is empty.");
    }
    spdlog::debug("Iterating through the AST tree with {} nodes.", current_ast_tree.size());
    // 3. start iterating through the AST tree
    recursive_iterate_ast_tree(current_ast_tree.begin());
    spdlog::info("Intermediate code generation completed successfully.");
    // 4. write the generated intermediate code to the output file
    std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> interm_code_list = logical_env_simulator.get_intermediate_code_list();
    if (interm_code_list.empty()) {
        spdlog::warn("No intermediate code generated. The list is empty.");
    }
    int index = 0;
    // if file exists, overwrite it
    std::ofstream output_stream(output_file);
    if (!output_stream.is_open()) {
        spdlog::error("Failed to open output file: {}", output_file);
        throw std::runtime_error("Failed to open output file.");
    }
    for (const auto& code : interm_code_list) {
        // Format the index with a fixed width, e.g., 3 characters, left-aligned
        output_stream << std::left << std::setw(3) << index++ << ". " << code->toString() << "\n";
        spdlog::debug("Written intermediate code {}: {}", index - 1, code->toString());
    }
}

void IntermCodeGenerator::recursive_iterate_ast_tree(
    tree<std::shared_ptr<ast_model::ASTNodeContent>>::iterator current_node
) {
    // much alike the semantic analysis, we will iterate through the AST tree
    // check children
    int children = current_node.number_of_children();

    for (int i = 0; i < children; ++i) {
        // get the child node
        auto child_it = current_ast_tree.child(current_node, i);
        // iterate recursively
        recursive_iterate_ast_tree(child_it);
    }

    // post-order processing of the current node
    current_node.node->data->generate_intermediate_code(logical_env_simulator);
    spdlog::debug("Generated intermediate code for node: {}", current_node.node->data->to_string());

    return; // return to the caller
}