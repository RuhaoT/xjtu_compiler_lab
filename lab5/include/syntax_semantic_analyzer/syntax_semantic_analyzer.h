#ifndef SYNTAX_SEMANTIC_ANALYZER_H
#define SYNTAX_SEMANTIC_ANALYZER_H

#include "ast_model.h"
#include "scope_table.h"
#include "lr_parsing_model.h"
#include "cfg_model.h"
#include "symbol_table.h"
#include "syntax_semantic_model.h"
#include "tree.hh"
#include "spdlog/spdlog.h"
#include "token_model.h"

namespace syntax_semantic_analyzer {
    struct analysis_result {
        tree<std::shared_ptr<ast_model::ASTNodeContent>> ast_tree; // AST树
        SymbolTable symbol_table; // 符号表
        ScopeTable scope_table; // 作用域表
    };

    // create a new ASTNodeContent shared_ptr from the given node type and related information
    std::shared_ptr<ast_model::ASTNodeContent> create_ast_node(
        const std::string& node_type,
        const std::string& node_value = ""
    );
};

class SyntaxSemanticAnalyzer {
public:
    SyntaxSemanticAnalyzer(); // Constructor to initialize member variables

    
    void prepair_new_analysis(
        const lr_parsing_model::LRParsingTable& slr1_parsing_table,
        const syntax_semantic_model::ProductionInfoMapping& production_info_mapping,
        const std::vector<Token>& tokens
    );

    // Analyze the syntax and semantics of the given AST
    syntax_semantic_analyzer::analysis_result analyze_syntax_semantics(
        const lr_parsing_model::LRParsingTable& slr1_parsing_table,
        const syntax_semantic_model::ProductionInfoMapping& production_info_mapping,
        const std::vector<Token>& tokens
    );

    // get blank AST tree after syntax analysis
    tree<std::shared_ptr<ast_model::ASTNodeContent>> get_blank_ast_tree();

    // reset all member variables to their initial state
    void reset();

    // check if the toke stream and info_mapping are valid
    void input_check();

    // syntax analysis & build empty AST tree
    void syntax_anlaysis();

    // semantic analysis
    void semantic_analysis();

    // recursively iterate through the AST tree and perform semantic actions
    void recursive_iterate_ast_tree(
        tree<std::shared_ptr<ast_model::ASTNodeContent>>::iterator node
    );

public:
    // for debugging purposes, set all member variables as public
    std::vector<cfg_model::symbol> current_symbol_stack; // 当前符号栈
    std::vector<std::string> current_state_stack; // 当前状态栈
    std::vector<tree<std::shared_ptr<ast_model::ASTNodeContent>>> current_ast_subtree_stack; // 当前AST子树栈
    int current_token_index = 0; // 当前token索引
    tree<std::shared_ptr<ast_model::ASTNodeContent>> current_ast_tree; // 当前AST树
    std::shared_ptr<SymbolTable> symbol_table; // 符号表
    std::shared_ptr<ScopeTable> scope_table; // 作用域表

    // SLR(1) parsing table
    lr_parsing_model::LRParsingTable slr1_parsing_table_ref; // SLR(1)分析表
    // production info mapping
    syntax_semantic_model::ProductionInfoMapping production_info_mapping_ref; // 产生式信息映射
    // token stream
    std::vector<Token> tokens_ref; // token流
};

#endif // !SYNTAX_SEMANTIC_ANALYZER_H