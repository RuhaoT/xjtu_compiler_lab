#ifndef INTERM_CODE_GENERATOR_H
#define INTERM_CODE_GENERATOR_H

#include "interm_code/interm_code_model.h"
#include "logical_env_simulator.h"
#include "symbol_table.h"
#include "syntax_semantic_model.h"
#include "ast_model.h"
#include "tree/tree.hh"
#include <memory>
#include <string>

class IntermCodeGenerator {
public:
    LogicalEnvSimulator logical_env_simulator; // Logical environment simulator
    tree<std::shared_ptr<ast_model::ASTNodeContent>> current_ast_tree; // Current AST tree being processed

public:
    IntermCodeGenerator() = default;
    IntermCodeGenerator(const SymbolTable& symbol_table, tree<std::shared_ptr<ast_model::ASTNodeContent>> ast_tree);
    
    void produce_intermediate_code(const std::string& output_file = "temp_intermediate_code.txt");

    void recursive_iterate_ast_tree(
        tree<std::shared_ptr<ast_model::ASTNodeContent>>::iterator current_node
    );
};

#endif // !INTERM_CODE_GENERATOR_H