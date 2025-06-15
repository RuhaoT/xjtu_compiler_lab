#include "syntax_semantic_analyzer.h"
#include "ast_model.h"
#include "scope_table.h"
#include "tree.hh"
#include "spdlog/spdlog.h"

std::shared_ptr<ast_model::ASTNodeContent> syntax_semantic_analyzer::create_ast_node(
    const std::string& node_type,
    const std::string& node_value
) {
    // a lot of cases
    // terminals with values
    if (node_type == "ID"||
        node_type == "NUM"||
        node_type == "FLO"||
        node_type == "ROP"
    )
    {
        return std::make_shared<ast_model::TerminalNode>(node_type, node_value);
    }
    // terminals without values
    else if (node_type == "ADD"||
             node_type == "MUL"||
             node_type == "ASG"||
             node_type == "LPA"||
             node_type == "RPA"||
             node_type == "LBK"||
             node_type == "RBK"||
             node_type == "LBR"||
             node_type == "RBR"||
             node_type == "CMA"||
             node_type == "SCO"||
             node_type == "INT"||
             node_type == "FLOAT"||
             node_type == "IF"||
             node_type == "ELSE"||
             node_type == "WHILE"||
             node_type == "RETURN"||
             node_type == "INPUT"||
             node_type == "VOID")
    {
        return std::make_shared<ast_model::TerminalNode>(node_type, "");
    }
    // non-terminals
    // program
    else if (node_type == "PROGRAM") {
        return std::make_shared<ast_model::ProgramNode>();
    }
    // declaration list
    else if (node_type == "DECL_LIST") {
        return std::make_shared<ast_model::DeclListNode>();
    }
    // different types of declarations
    else if (node_type == "DECL_VAR" ||
        node_type == "DECL_FUNC" ||
        node_type == "DECL_ARRAY") {
        return std::make_shared<ast_model::DeclNode>(node_type);
        }
    // type node
    else if (node_type == "TYPE") {
        return std::make_shared<ast_model::TypeNode>();
    }
    // argument list
    else if (node_type == "ARG_LIST") {
        return std::make_shared<ast_model::ArgListNode>();
    }
    // argument nodes
    else if (node_type == "ARG_VAR" ||
             node_type == "ARG_FUNC" ||
             node_type == "ARG_ARRAY") {
        return std::make_shared<ast_model::ArgNode>(node_type);
    }
    // statement list
    else if (node_type == "STAT_LIST") {
        return std::make_shared<ast_model::StatListNode>();
    }
    // different types of statements
    else if (node_type == "STAT_ASSIGN" ||
             node_type == "STAT_ARRAY_ASSIGN" ||
             node_type == "STAT_IF" ||
             node_type == "STAT_IF_ELSE" ||
             node_type == "STAT_WHILE" ||
             node_type == "STAT_RETURN" ||
             node_type == "STAT_COMPOUND" ||
             node_type == "STAT_FUNC_CALL") {
        return std::make_shared<ast_model::StatNode>(node_type);
    }
    // expression nodes
    else if (node_type == "EXPR_CONST" ||
             node_type == "EXPR_VAR" ||
             node_type == "EXPR_FUNC" ||
             node_type == "EXPR_ARRAY" ||
             node_type == "EXPR_ARITH_NOCONST" ||
             node_type == "EXPR_PAREN_NOCONST" ||
             node_type == "EXPR_MUL_TEMP" ||
             node_type == "EXPR_ATOMIC_TEMP") {
        return std::make_shared<ast_model::ExprNode>(node_type);
    }
    // boolean nodes
    else if (node_type == "BOOL_OP" ||
             node_type == "BOOL_EXPR") {
        return std::make_shared<ast_model::BoolNode>(node_type);
    }
    // real arg list node
    else if (node_type == "RARG_LIST") {
        return std::make_shared<ast_model::RealArgListNode>();
    }
    // real argument nodes
    else if (node_type == "RARG_EXPR" ||
             node_type == "RARG_FUNC" ||
             node_type == "RARG_ARRAY") {
        return std::make_shared<ast_model::RealArgNode>(node_type);
    }
    else {
        spdlog::error("Unknown ASTNodeType: {}", node_type);
        throw std::invalid_argument("Unknown ASTNodeType: " + node_type);
    }
    return nullptr; // This should never be reached, but added to avoid compiler warnings
}



SyntaxSemanticAnalyzer::SyntaxSemanticAnalyzer() {
    reset();
};

void SyntaxSemanticAnalyzer::reset() {
    current_symbol_stack.clear();
    current_state_stack.clear();
    current_ast_subtree_stack.clear();
    current_token_index = 0;
    symbol_table = std::make_shared<SymbolTable>();
    scope_table = std::make_shared<ScopeTable>();
}

void SyntaxSemanticAnalyzer::prepair_new_analysis(
    const lr_parsing_model::LRParsingTable& slr1_parsing_table,
    const syntax_semantic_model::ProductionInfoMapping& production_info_mapping,
    const std::vector<Token>& tokens
) {
    reset();
    slr1_parsing_table_ref = slr1_parsing_table; // Store the parsing table
    production_info_mapping_ref = production_info_mapping; // Store the production info mapping
    tokens_ref = tokens; // Store the token stream
    input_check(); // Check the input validity
    
    return;
}

syntax_semantic_analyzer::analysis_result SyntaxSemanticAnalyzer::analyze_syntax_semantics(
        const lr_parsing_model::LRParsingTable& slr1_parsing_table,
        const syntax_semantic_model::ProductionInfoMapping& production_info_mapping,
        const std::vector<Token>& tokens
){
    prepair_new_analysis(slr1_parsing_table, production_info_mapping, tokens);
    syntax_anlaysis(); // Perform syntax analysis and build the AST tree
    semantic_analysis(); // Perform semantic analysis on the AST tree

    // Prepare the result
    syntax_semantic_analyzer::analysis_result result;
    result.ast_tree = current_ast_tree;
    result.symbol_table = *symbol_table;
    result.scope_table = *scope_table;

    spdlog::info("Syntax and semantic analysis completed successfully.");
    return result;
}

// get blank AST tree after syntax analysis
tree<std::shared_ptr<ast_model::ASTNodeContent>> SyntaxSemanticAnalyzer::get_blank_ast_tree(){
    syntax_anlaysis();

    return current_ast_tree; // Return the blank AST tree after syntax analysis
}

void SyntaxSemanticAnalyzer::input_check() {
    if (tokens_ref.empty()) {
        spdlog::error("Token stream is empty.");
        throw std::runtime_error("Token stream is empty.");
    }
    if (production_info_mapping_ref.production_info.empty()) {
        spdlog::error("Production info mapping is empty.");
        throw std::runtime_error("Production info mapping is empty.");
    }
    if (!slr1_parsing_table_ref.filling_check()) {
        spdlog::error("SLR(1) parsing table is not filled correctly.");
        throw std::runtime_error("SLR(1) parsing table is not filled correctly.");
    }

    // check if each token type could be translated to parsing table symbol
    std::unordered_set<cfg_model::symbol> parsing_table_terminals;
    for (const auto& symbol : slr1_parsing_table_ref.all_symbols) {
        if (symbol.is_terminal) {
            parsing_table_terminals.insert(symbol);
        }
    }
    for (const auto& token : tokens_ref) {
        cfg_model::symbol token_symbol;
        token_symbol.name = token.type;
        token_symbol.is_terminal = true; // Tokens are always terminal symbols
        token_symbol.special_property = ""; // No special property for tokens
        if (parsing_table_terminals.find(token_symbol) == parsing_table_terminals.end()) {
            spdlog::error("Token type '{}' is not found in the parsing table symbols.", token.type);
            throw std::runtime_error("Token type is not found in the parsing table symbols.");
        }
    }
    spdlog::debug("Token check passed. All tokens are valid.");
    // check if all token type could be translated to ast_model::ASTNodeType
    for (const auto& token : tokens_ref) {
        try {
            ast_model::string_to_ast_node_type(token.type);
        } catch (const std::exception& e) {
            spdlog::error("Token type '{}' cannot be translated to ASTNodeType: {}", token.type, e.what());
            throw std::runtime_error("Token type cannot be translated to ASTNodeType.");
        }
    }
    spdlog::debug("Token type check passed. All token types are valid ASTNodeTypes.");
    // check if all production info could be translated to ASTNodeType
    for (const auto& [lhs, rhs_map] : production_info_mapping_ref.production_info) {
        for (const auto& [rhs, node_type] : rhs_map) {
            try {
                ast_model::string_to_ast_node_type(node_type);
            } catch (const std::exception& e) {
                spdlog::error("Production info '{}' -> '{}' cannot be translated to ASTNodeType: {}", lhs.name, node_type, e.what());
                throw std::runtime_error("Production info cannot be translated to ASTNodeType.");
            }
        }
    }
    spdlog::debug("Production info check passed. All production info are valid ASTNodeTypes.");
}

void SyntaxSemanticAnalyzer::syntax_anlaysis() {
    spdlog::info("Starting syntax analysis...");
    // Initialize the parsing stacks
    current_symbol_stack.clear();
    current_state_stack.clear();
    current_ast_subtree_stack.clear();
    
    // Initialize the start state and symbol stack
    current_state_stack.push_back(slr1_parsing_table_ref.start_state);
    
    // find the end symbol in the parsing table
    cfg_model::symbol end_symbol;
    bool end_symbol_found = false;
    for (const auto& symbol : slr1_parsing_table_ref.all_symbols) {
        if (symbol.special_property == "END") {
            end_symbol = symbol;
            end_symbol_found = true;
            break;
        }
    }
    if (!end_symbol_found) {
        spdlog::error("End symbol not found in the parsing table.");
        throw std::runtime_error("End symbol not found in the parsing table.");
    }
    Token end_token;
    end_token.type = end_symbol.name;
    end_token.value = ""; // End token has no value
    tokens_ref.push_back(end_token); // Add the end token to the token stream

    // start parsing
    bool accepted = false;
    while (current_token_index < tokens_ref.size()) {
        // Get the current token
        const Token& current_token = tokens_ref[current_token_index];
        cfg_model::symbol current_symbol;
        // lookup the current token in the parsing table using token type
        bool symbol_found = false;
        for (const auto& symbol : slr1_parsing_table_ref.all_symbols) {
            if (symbol.name == current_token.type && symbol.is_terminal) {
                current_symbol = symbol;
                symbol_found = true;
                break;
            }
        }
        if (!symbol_found) {
            spdlog::error("Current token '{}' is not found in the parsing table symbols.", current_token.type);
            throw std::runtime_error("Current token is not found in the parsing table symbols.");
        }
        spdlog::debug("Current token: {}, index: {}", current_token.type, current_token_index);

        // get the current action set using the current state and current symbol
        std::unordered_set<lr_parsing_model::Action> actions = slr1_parsing_table_ref.get_actions(current_state_stack.back(), current_symbol);
        if (actions.empty()) {
            spdlog::error("No actions found for state '{}' and symbol '{}'.", current_state_stack.back(), current_symbol.name);
            throw std::runtime_error("No actions found for current state and symbol.");
        }
        else if (actions.size() > 1) {
            spdlog::error("Multiple actions found for state '{}' and symbol '{}'.", current_state_stack.back(), current_symbol.name);
            throw std::runtime_error("Multiple actions found for current state and symbol.");
        }
        lr_parsing_model::Action current_action = *actions.begin();
        spdlog::debug("Current action: {}", current_action.action_type);
        // Perform the action based on its type
        if (current_action.action_type == "empty")
        {
            // parsing failed due to empty action
            spdlog::error("Parsing failed due to empty action for state '{}' and symbol '{}'.", current_state_stack.back(), current_symbol.name);
            throw std::runtime_error("Parsing failed due to empty action.");
        }
        else if (current_action.action_type == "shift")
        {
            // Shift action: push the current token onto the symbol stack and state stack
            current_symbol_stack.push_back(current_symbol);
            current_state_stack.push_back(current_action.target_state);
            // Create a new AST node for the current token
            auto ast_node = std::make_shared<ast_model::TerminalNode>(current_token.type, current_token.value);
            tree<std::shared_ptr<ast_model::ASTNodeContent>> ast_subtree;
            ast_subtree.set_head(ast_node);
            current_ast_subtree_stack.push_back(ast_subtree);
            spdlog::debug("Shifted token '{}' to state '{}'.", current_token.type, current_action.target_state);
            // Move to the next token
            current_token_index++;
        }
        else if (current_action.action_type == "reduce")
        {
            // Reduce action: pop symbols from the symbol stack according to the rule's right-hand side
            const auto& lhs = current_action.reduce_rule_lhs;
            const std::vector<cfg_model::symbol>& rhs = current_action.reduce_rule_rhs;
            std::string rhs_str;
            for (const auto& symbol : rhs) {
                rhs_str += symbol.name + " ";
            }
            spdlog::debug("Reduce action: LHS: {}, RHS: {}", lhs.name, rhs_str);
            
            int rhs_size = rhs.size();
            if (rhs_size > current_symbol_stack.size()) {
                spdlog::error("Not enough symbols in the stack to perform reduce action.");
                throw std::runtime_error("Not enough symbols in the stack to perform reduce action.");
            }
            // pop symbol stack & append lhs to the symbol stack
            current_symbol_stack.resize(current_symbol_stack.size() - rhs_size);
            current_symbol_stack.push_back(lhs);
            // pop state stack & append new state to the state stack
            current_state_stack.resize(current_state_stack.size() - rhs_size);
            std::unordered_set<std::string> new_states = slr1_parsing_table_ref.get_gotos(current_state_stack.back(), lhs);
            if (new_states.size() != 1) {
                spdlog::error("Reduce action for state '{}' and symbol '{}' resulted in multiple or no new states.", current_state_stack.back(), lhs.name);
                throw std::runtime_error("Reduce action resulted in multiple or no new states.");
            }
            std::string new_state = *new_states.begin();
            current_state_stack.push_back(new_state);
            // pop AST subtree stack & grazp the subtrees for the right-hand side
            // look up the production info to get the node type
            std::string node_type_str = production_info_mapping_ref.get_node_type(lhs, rhs);
            ast_model::ASTNodeType node_type = ast_model::string_to_ast_node_type(node_type_str);
            // create a new AST subtree
            tree<std::shared_ptr<ast_model::ASTNodeContent>> ast_subtree;
            auto ast_node = syntax_semantic_analyzer::create_ast_node(node_type_str);
            ast_subtree.set_head(ast_node);
        
            std::vector<tree<std::shared_ptr<ast_model::ASTNodeContent>>> rhs_subtrees;
            for (int i = 0; i < rhs_size; ++i) {
                if (current_ast_subtree_stack.empty()) {
                    spdlog::error("Not enough AST subtrees to perform reduce action.");
                    throw std::runtime_error("Not enough AST subtrees to perform reduce action.");
                }
                rhs_subtrees.push_back(current_ast_subtree_stack.back());
                current_ast_subtree_stack.pop_back();
            }
            // reverse the order of the subtrees
            std::reverse(rhs_subtrees.begin(), rhs_subtrees.end());
            // start grafting the subtrees
            std::vector<std::shared_ptr<ast_model::ASTNodeContent>> sub_ast_contents;
            int children_counter = 0;
            for (auto& subtree : rhs_subtrees) {
                sub_ast_contents.push_back(subtree.head->data);
                ast_subtree.move_in_as_nth_child(ast_subtree.begin(), children_counter, subtree);
                children_counter++;
            };
            // now we have the complete subtree for the current production, start taking in the subnodes
            ast_node->subnode_takein(sub_ast_contents);
            // push the new subtree to the AST subtree stack
            current_ast_subtree_stack.push_back(ast_subtree);
            spdlog::debug("Reduced to node type '{}'.", node_type_str);
        }
        else if (current_action.action_type == "accept")
        {
            // Accept action: parsing is successful
            accepted = true;
            spdlog::info("Parsing accepted.");
            break;
        }
        else
        {
            spdlog::error("Unknown action type: {}", current_action.action_type);
            throw std::runtime_error("Unknown action type.");
        }
    }

    // Check if parsing was accepted
    if (!accepted) {
        spdlog::error("Parsing failed. No accept action found.");
        throw std::runtime_error("Parsing failed. No accept action found.");
    }
    // extract the final AST subtree, which should be the only one left in the stack
    if (current_ast_subtree_stack.size() != 1) {
        spdlog::error("Parsing failed. Expected one AST subtree, but found {}.", current_ast_subtree_stack.size());
        throw std::runtime_error("Parsing failed. Expected one AST subtree.");
    }
    else{
        current_ast_tree = current_ast_subtree_stack.back();
    }
    spdlog::info("Syntax analysis completed successfully.");
}

void SyntaxSemanticAnalyzer::semantic_analysis() {
    spdlog::info("Starting semantic analysis...");
    // Start semantic analysis from the root of the AST
    if (current_ast_tree.empty()) {
        spdlog::error("AST tree is empty. Cannot perform semantic analysis.");
        throw std::runtime_error("AST tree is empty.");
    }
    spdlog::debug("Iterating through the AST tree with {} nodes.", current_ast_tree.size());
    // Prepair scope table and symbol table
    scope_table->reset(); // Reset the scope table
    // start iterating through the AST tree
    recursive_iterate_ast_tree(current_ast_tree.begin());
    spdlog::info("Semantic analysis completed successfully.");
}


void SyntaxSemanticAnalyzer::recursive_iterate_ast_tree(
    tree<std::shared_ptr<ast_model::ASTNodeContent>>::iterator current_node
)
{
    // check children
    int children = current_node.number_of_children();

    std::vector<std::shared_ptr<ast_model::ASTNodeContent>> subnodes;

    for (int i = 0; i < children; ++i) {
        // get the child node
        auto child_it = current_ast_tree.child(current_node, i);

        // add the child node to the current node's subnodes
        subnodes.push_back(child_it.node->data);

        if (current_node.node->data->node_type == ast_model::ASTNodeType::DECL_FUNC && child_it.node->data->node_type == ast_model::ASTNodeType::DECL_LIST) {
            // if the current node is a function declaration and the child just visited is an declaration list, it means this function is being declared, now it's time to enter a new function scope
            scope_table->enterNewScope();
        }

        // recursively iterate through the child node
        recursive_iterate_ast_tree(child_it);

        if (current_node.node->data->node_type == ast_model::ASTNodeType::DECL_FUNC && child_it.node->data->node_type == ast_model::ASTNodeType::STAT_LIST) {
            // if the current node is a function declaration and the child just visited is a statement list, it means this function has been fully processed, now it's time to return to the parent function scope
            scope_table->exitCurrentScope();
        }
    }

    // after all children are processed, perform the semantic action for the current node
    // !!! IF THIS IS A LEAF NODE, IT MEANS THIS IS A TERMINAL NODE, AND WE SHOULD NOT CALL THE SEMANTIC ACTION
    if (children == 0) {
        spdlog::debug("Current node is a leaf node, skipping semantic action.");
        return;
    }
    // 1. subnode takein 2. semantic action
    // take in the subnodes for the current node
    current_node.node->data->subnode_takein(subnodes);
    spdlog::debug("Taking in {} subnodes for node type: {}", subnodes.size(), ast_model::ast_node_type_to_string(current_node.node->data->node_type));
    current_node.node->data->semantic_action(
        scope_table->getCurrentScope(),
        symbol_table,
        scope_table
    );
    spdlog::debug("Processed AST node: {}", current_node.node->data->to_string());

    return;
}