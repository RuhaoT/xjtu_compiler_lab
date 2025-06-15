#include "ast_model.h"
#include "symbol_table.h"
#include "scope_table.h"
#include "spdlog/spdlog.h"

// string <-> ASTNodeType conversion functions
namespace ast_model
{
    ASTNodeType string_to_ast_node_type(const std::string &node_type)
    {
        if (node_type == "PROGRAM")
            return ASTNodeType::PROGRAM;
        else if (node_type == "DECL_LIST")
            return ASTNodeType::DECL_LIST;
        else if (node_type == "DECL_VAR")
            return ASTNodeType::DECL_VAR;
        else if (node_type == "DECL_FUNC")
            return ASTNodeType::DECL_FUNC;
        else if (node_type == "DECL_ARRAY")
            return ASTNodeType::DECL_ARRAY;
        else if (node_type == "TYPE")
            return ASTNodeType::TYPE;
        else if (node_type == "ARG_LIST")
            return ASTNodeType::ARG_LIST;
        else if (node_type == "ARG_VAR")
            return ASTNodeType::ARG_VAR;
        else if (node_type == "ARG_FUNC")
            return ASTNodeType::ARG_FUNC;
        else if (node_type == "ARG_ARRAY")
            return ASTNodeType::ARG_ARRAY;
        else if (node_type == "STAT_LIST")
            return ASTNodeType::STAT_LIST;
        else if (node_type == "STAT_ASSIGN")
            return ASTNodeType::STAT_ASSIGN;
        else if (node_type == "STAT_ARRAY_ASSIGN")
            return ASTNodeType::STAT_ARRAY_ASSIGN;
        else if (node_type == "STAT_IF")
            return ASTNodeType::STAT_IF;
        else if (node_type == "STAT_IF_ELSE")
            return ASTNodeType::STAT_IF_ELSE;
        else if (node_type == "STAT_WHILE")
            return ASTNodeType::STAT_WHILE;
        else if (node_type == "STAT_RETURN")
            return ASTNodeType::STAT_RETURN;
        else if (node_type == "STAT_COMPOUND")
            return ASTNodeType::STAT_COMPOUND;
        else if (node_type == "STAT_FUNC_CALL")
            return ASTNodeType::STAT_FUNC_CALL;
        else if (node_type == "STAT_IFELSE_MS")
            return ASTNodeType::STAT_IFELSE_MS;
        else if (node_type == "STAT_IFELSE_UMS")
            return ASTNodeType::STAT_IFELSE_UMS;
        else if (node_type == "STAT_IF_ELSE_CHAIN")
            return ASTNodeType::STAT_IF_ELSE_CHAIN;
        else if (node_type == "EXPR_CONST")
            return ASTNodeType::EXPR_CONST;
        else if (node_type == "EXPR_VAR")
            return ASTNodeType::EXPR_VAR;
        else if (node_type == "EXPR_FUNC")
            return ASTNodeType::EXPR_FUNC;
        else if (node_type == "EXPR_ARRAY")
            return ASTNodeType::EXPR_ARRAY;
        else if (node_type == "EXPR_ARITH_NOCONST")
            return ASTNodeType::EXPR_ARITH_NOCONST;
        else if (node_type == "EXPR_PAREN_NOCONST")
            return ASTNodeType::EXPR_PAREN_NOCONST;
        else if (node_type == "EXPR_MUL_TEMP")
            return ASTNodeType::EXPR_MUL_TEMP;
        else if (node_type == "EXPR_ATOMIC_TEMP")
            return ASTNodeType::EXPR_ATOMIC_TEMP;
        else if (node_type == "BOOL_OP")
            return ASTNodeType::BOOL_OP;
        else if (node_type == "BOOL_EXPR")
            return ASTNodeType::BOOL_EXPR;
        else if (node_type == "RARG_EXPR")
            return ASTNodeType::RARG_EXPR;
        else if (node_type == "RARG_FUNC")
            return ASTNodeType::RARG_FUNC;
        else if (node_type == "RARG_ARRAY")
            return ASTNodeType::RARG_ARRAY;
        else if (node_type == "RARG_LIST")
            return ASTNodeType::RARG_LIST;
        else if (node_type == "ID")
            return ASTNodeType::ID;
        else if (node_type == "NUM")
            return ASTNodeType::NUM;
        else if (node_type == "FLO")
            return ASTNodeType::FLO;
        else if (node_type == "ADD")
            return ASTNodeType::ADD;
        else if (node_type == "MUL")
            return ASTNodeType::MUL;
        else if (node_type == "ROP")
            return ASTNodeType::ROP;
        else if (node_type == "ASG")
            return ASTNodeType::ASG;
        else if (node_type == "LPA")
            return ASTNodeType::LPA;
        else if (node_type == "RPA")
            return ASTNodeType::RPA;
        else if (node_type == "LBK")
            return ASTNodeType::LBK;
        else if (node_type == "RBK")
            return ASTNodeType::RBK;
        else if (node_type == "LBR")
            return ASTNodeType::LBR;
        else if (node_type == "RBR")
            return ASTNodeType::RBR;
        else if (node_type == "CMA")
            return ASTNodeType::CMA;
        else if (node_type == "SCO")
            return ASTNodeType::SCO;
        else if (node_type == "INT")
            return ASTNodeType::INT;
        else if (node_type == "FLOAT")
            return ASTNodeType::FLOAT;
        else if (node_type == "IF")
            return ASTNodeType::IF;
        else if (node_type == "ELSE")
            return ASTNodeType::ELSE;
        else if (node_type == "WHILE")
            return ASTNodeType::WHILE;
        else if (node_type == "RETURN")
            return ASTNodeType::RETURN;
        else if (node_type == "INPUT")
            return ASTNodeType::INPUT;
        else if (node_type == "VOID")
            return ASTNodeType::VOID;
        else
        {
            throw std::invalid_argument("Unknown ASTNodeType string: " + node_type);
        }
    }

    std::string ast_node_type_to_string(ASTNodeType node_type)
    {
        switch (node_type)
        {
        case ASTNodeType::PROGRAM:
            return "PROGRAM";
        case ASTNodeType::DECL_LIST:
            return "DECL_LIST";
        case ASTNodeType::DECL_VAR:
            return "DECL_VAR";
        case ASTNodeType::DECL_FUNC:
            return "DECL_FUNC";
        case ASTNodeType::DECL_ARRAY:
            return "DECL_ARRAY";
        case ASTNodeType::TYPE:
            return "TYPE";
        case ASTNodeType::ARG_LIST:
            return "ARG_LIST";
        case ASTNodeType::ARG_VAR:
            return "ARG_VAR";
        case ASTNodeType::ARG_FUNC:
            return "ARG_FUNC";
        case ASTNodeType::ARG_ARRAY:
            return "ARG_ARRAY";
        case ASTNodeType::STAT_LIST:
            return "STAT_LIST";
        case ASTNodeType::STAT_ASSIGN:
            return "STAT_ASSIGN";
        case ASTNodeType::STAT_ARRAY_ASSIGN:
            return "STAT_ARRAY_ASSIGN";
        case ASTNodeType::STAT_IF:
            return "STAT_IF";
        case ASTNodeType::STAT_IF_ELSE:
            return "STAT_IF_ELSE";
        case ASTNodeType::STAT_WHILE:
            return "STAT_WHILE";
        case ASTNodeType::STAT_RETURN:
            return "STAT_RETURN";
        case ASTNodeType::STAT_COMPOUND:
            return "STAT_COMPOUND";
        case ASTNodeType::STAT_FUNC_CALL:
            return "STAT_FUNC_CALL";
        case ASTNodeType::STAT_IFELSE_MS:
            return "STAT_IFELSE_MS";
        case ASTNodeType::STAT_IFELSE_UMS:
            return "STAT_IFELSE_UMS";
        case ASTNodeType::STAT_IF_ELSE_CHAIN:
            return "STAT_IF_ELSE_CHAIN";
        case ASTNodeType::EXPR_CONST:
            return "EXPR_CONST";
        case ASTNodeType::EXPR_VAR:
            return "EXPR_VAR";
        case ASTNodeType::EXPR_FUNC:
            return "EXPR_FUNC";
        case ASTNodeType::EXPR_ARRAY:
            return "EXPR_ARRAY";
        case ASTNodeType::EXPR_ARITH_NOCONST:
            return "EXPR_ARITH_NOCONST";
        case ASTNodeType::EXPR_PAREN_NOCONST:
            return "EXPR_PAREN_NOCONST";
        case ASTNodeType::EXPR_MUL_TEMP:
            return "EXPR_MUL_TEMP";
        case ASTNodeType::EXPR_ATOMIC_TEMP:
            return "EXPR_ATOMIC_TEMP";
        case ASTNodeType::BOOL_OP:
            return "BOOL_OP";
        case ASTNodeType::BOOL_EXPR:
            return "BOOL_EXPR";
        case ASTNodeType::RARG_EXPR:
            return "RARG_EXPR";
        case ASTNodeType::RARG_FUNC:
            return "RARG_FUNC";
        case ASTNodeType::RARG_ARRAY:
            return "RARG_ARRAY";
        case ASTNodeType::RARG_LIST:
            return "RARG_LIST";
        case ASTNodeType::ID:
            return "ID";
        case ASTNodeType::NUM:
            return "NUM";
        case ASTNodeType::FLO:
            return "FLO";
        case ASTNodeType::ADD:
            return "ADD";
        case ASTNodeType::MUL:
            return "MUL";
        case ASTNodeType::ROP:
            return "ROP";
        case ASTNodeType::ASG:
            return "ASG";
        case ASTNodeType::LPA:
            return "LPA";
        case ASTNodeType::RPA:
            return "RPA";
        case ASTNodeType::LBK:
            return "LBK";
        case ASTNodeType::RBK:
            return "RBK";
        case ASTNodeType::LBR:
            return "LBR";
        case ASTNodeType::RBR:
            return "RBR";
        case ASTNodeType::CMA:
            return "CMA";
        case ASTNodeType::SCO:
            return "SCO";
        case ASTNodeType::INT:
            return "INT";
        case ASTNodeType::FLOAT:
            return "FLOAT";
        case ASTNodeType::IF:
            return "IF";
        case ASTNodeType::ELSE:
            return "ELSE";
        case ASTNodeType::WHILE:
            return "WHILE";
        case ASTNodeType::RETURN:
            return "RETURN";
        case ASTNodeType::INPUT:
            return "INPUT";
        case ASTNodeType::VOID:
            return "VOID";
        }
        throw std::invalid_argument("Unknown ASTNodeType enum value");
    };

    std::shared_ptr<void> cast_ast_node_base_ptr_to_specific_node(
        const std::shared_ptr<ASTNodeContent> &node,
        const std::string &node_type)
    {
        // a lot of cases
        // terminals w/o values
        if (node_type == "ID" ||
            node_type == "NUM" ||
            node_type == "FLO" ||
            node_type == "ROP" ||
            node_type == "ADD" ||
            node_type == "MUL" ||
            node_type == "ASG" ||
            node_type == "LPA" ||
            node_type == "RPA" ||
            node_type == "LBK" ||
            node_type == "RBK" ||
            node_type == "LBR" ||
            node_type == "RBR" ||
            node_type == "CMA" ||
            node_type == "SCO" ||
            node_type == "INT" ||
            node_type == "FLOAT" ||
            node_type == "IF" ||
            node_type == "ELSE" ||
            node_type == "WHILE" ||
            node_type == "RETURN" ||
            node_type == "INPUT" ||
            node_type == "VOID")
        {
            return std::dynamic_pointer_cast<ast_model::TerminalNode>(node);
        }
        else if (node_type == "PROGRAM")
        {
            return std::dynamic_pointer_cast<ast_model::ProgramNode>(node);
        }
        else if (node_type == "DECL_LIST")
        {
            return std::dynamic_pointer_cast<ast_model::DeclListNode>(node);
        }
        else if (node_type == "DECL_VAR" ||
                 node_type == "DECL_FUNC" ||
                 node_type == "DECL_ARRAY")
        {
            return std::dynamic_pointer_cast<ast_model::DeclNode>(node);
        }
        else if (node_type == "TYPE")
        {
            return std::dynamic_pointer_cast<ast_model::TypeNode>(node);
        }
        else if (node_type == "ARG_LIST")
        {
            return std::dynamic_pointer_cast<ast_model::ArgListNode>(node);
        }
        else if (node_type == "ARG_VAR" ||
                 node_type == "ARG_FUNC" ||
                 node_type == "ARG_ARRAY")
        {
            return std::dynamic_pointer_cast<ast_model::ArgNode>(node);
        }
        else if (node_type == "STAT_LIST")
        {
            return std::dynamic_pointer_cast<ast_model::StatListNode>(node);
        }
        else if (node_type == "STAT_ASSIGN" ||
                 node_type == "STAT_ARRAY_ASSIGN" ||
                 node_type == "STAT_IF" ||
                 node_type == "STAT_IF_ELSE" ||
                 node_type == "STAT_WHILE" ||
                 node_type == "STAT_RETURN" ||
                 node_type == "STAT_COMPOUND" ||
                 node_type == "STAT_FUNC_CALL" ||
                 node_type == "STAT_IFELSE_MS" ||
                 node_type == "STAT_IFELSE_UMS" ||
                 node_type == "STAT_IF_ELSE_CHAIN")
        {
            return std::dynamic_pointer_cast<ast_model::StatNode>(node);
        }
        else if (node_type == "EXPR_CONST" ||
                 node_type == "EXPR_VAR" ||
                 node_type == "EXPR_FUNC" ||
                 node_type == "EXPR_ARRAY" ||
                 node_type == "EXPR_ARITH_NOCONST" ||
                 node_type == "EXPR_PAREN_NOCONST" ||
                 node_type == "EXPR_MUL_TEMP" ||
                 node_type == "EXPR_ATOMIC_TEMP")
        {
            return std::dynamic_pointer_cast<ast_model::ExprNode>(node);
        }
        else if (node_type == "BOOL_OP" ||
                 node_type == "BOOL_EXPR")
        {
            return std::dynamic_pointer_cast<ast_model::BoolNode>(node);
        }
        else if (node_type == "RARG_LIST")
        {
            return std::dynamic_pointer_cast<ast_model::RealArgListNode>(node);
        }
        else if (node_type == "RARG_EXPR" ||
                 node_type == "RARG_FUNC" ||
                 node_type == "RARG_ARRAY")
        {
            return std::dynamic_pointer_cast<ast_model::RealArgNode>(node);
        }
        else
        {
            throw std::invalid_argument("Unknown ASTNodeType string: " + node_type);
        }
    }

} // namespace ast_model

// all semantic actions
void ast_model::ProgramNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for ProgramNode with scope_id: {}", scope_id);
    // Implement semantic actions for ProgramNode
    // 1. check for main function with scope_id 0 in symbol_table
    if (symbol_table->findSymbolInScope("main", 0))
    {
        spdlog::info("Main function found in scope 0.");
    }
    else
    {
        spdlog::error("Main function not found in scope 0.");
        throw std::runtime_error("Main function not found in scope 0.");
    }
    // 2. check if the return type of main function is int
    std::optional<syntax_semantic_model::SymbolEntry> main_symbol_entry = symbol_table->findSymbolInScope("main", 0);
    if (main_symbol_entry->data_type != ast_node_type_to_string(ASTNodeType::INT))
    {
        spdlog::error("Main function return type is not int.");
        throw std::runtime_error("Main function return type is not int.");
    }
    // 3. check if the main function has no parameters
    if (main_symbol_entry->arg_list.has_value() && !main_symbol_entry->arg_list->empty())
    {
        spdlog::error("Main function should not have parameters.");
        throw std::runtime_error("Main function should not have parameters.");
    }
};

void ast_model::DeclListNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for DeclListNode with scope_id: {}", scope_id);
    this->scope_id = scope_id; // Set the scope_id for the declaration list node
    // Implement semantic actions for DeclListNode
    // if subnodes are empty, return
    if (subnodes.empty())
    {
        spdlog::debug("DeclListNode has no declarations.");
        this->decl_count = 0;
        return;
    }
    else
    {
        // dymanic cast subnode 0 to DeclListNode
        auto decl_list_node = std::dynamic_pointer_cast<DeclListNode>(subnodes[0]);
        // dymanic cast subnode 1 to DeclNode
        auto next_decl_node = std::dynamic_pointer_cast<DeclNode>(subnodes[1]);

        this->decl_count = decl_list_node->decl_count + 1;
        this->declarations = decl_list_node->declarations;
        this->declarations.push_back(next_decl_node);
    }
};

void ast_model::DeclNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for DeclNode with scope_id: {}", scope_id);
    this->scope_id = scope_id; // Set the scope_id for the declaration node
    // Implement semantic actions for DeclNode
    // 1. dymanic cast subnode 0 to TypeNode
    auto type_node = std::dynamic_pointer_cast<TypeNode>(subnodes[0]);
    // get data_type from type_node
    this->data_type = type_node->data_type;
    spdlog::debug("Data type for declaration node: {}", this->data_type);
    // 2. check if the data_type is valid
    if (this->node_type != ASTNodeType::DECL_FUNC && this->data_type == "VOID")
    {
        spdlog::error("Variable declaration cannot have VOID type.");
        throw std::runtime_error("Variable declaration cannot have VOID type.");
    };
    // 3. dymanic cast subnode 1 to IDNode and get var_name
    auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[1]);
    this->var_name = id_node->value;
    // 4. check if the variable already exists in the current scope
    if (symbol_table->symbolExists(this->var_name, scope_id))
    {
        spdlog::error("Variable '{}' already exists in scope {}.", this->var_name, scope_id);
        throw std::runtime_error("Variable already exists in current scope.");
    }
    // 5. if is normal variable declaration, add to symbol table
    if (this->node_type == ASTNodeType::DECL_VAR)
    {
        syntax_semantic_model::SymbolEntry symbol_entry;
        symbol_entry.symbol_name = this->var_name;
        symbol_entry.data_type = this->data_type;
        symbol_entry.symbol_type = syntax_semantic_model::SymbolType::Variable; // Set symbol type to Variable
        symbol_entry.scope_id = scope_id;
        symbol_entry.memory_size = (this->data_type == "INT") ? INT_MEMORY_SIZE : FLOAT_MEMORY_SIZE; // Set memory size based on data type
        symbol_table->addSymbol(symbol_entry);
        spdlog::info("Added variable '{}' of type '{}' to symbol table in scope {}.", this->var_name, this->data_type, scope_id);
    }
    else if (this->node_type == ASTNodeType::DECL_ARRAY)
    {
        // 6. if is array declaration, check if array_length is set
        if (!this->array_length.has_value())
        {
            spdlog::error("Array declaration '{}' must have a length.", this->var_name);
            throw std::runtime_error("Array declaration must have a length.");
        }
        // 7. add array to symbol table
        syntax_semantic_model::SymbolEntry symbol_entry;
        symbol_entry.symbol_name = this->var_name;
        symbol_entry.data_type = this->data_type;
        symbol_entry.symbol_type = syntax_semantic_model::SymbolType::Array; // Set symbol type to Array
        symbol_entry.scope_id = scope_id;
        symbol_entry.memory_size = INT_MEMORY_SIZE * this->array_length.value(); // Set memory size based on array length
        symbol_entry.array_length = this->array_length;                          // Set array length
        symbol_table->addSymbol(symbol_entry);
        spdlog::info("Added array '{}' of type '{}' with length {} to symbol table in scope {}.", this->var_name, this->data_type, this->array_length.value(), scope_id);
    }
    else if (this->node_type == ASTNodeType::DECL_FUNC)
    {
        // 8. if is function declaration, check if arg_list is set
        if (!this->arg_list.has_value())
        {
            spdlog::warn("Function '{}' has no arguments.", this->var_name);
        }
        // 9. add function to symbol table
        // dynamic cast subnode 7 to DeclListNode
        auto decl_list_node = std::dynamic_pointer_cast<DeclListNode>(this->subnodes[6]);
        syntax_semantic_model::SymbolEntry symbol_entry;
        symbol_entry.symbol_name = this->var_name;
        symbol_entry.symbol_type = syntax_semantic_model::SymbolType::Function; // Set symbol type to Function
        symbol_entry.data_type = this->data_type;
        symbol_entry.scope_id = scope_id;
        symbol_entry.memory_size = 0; // Functions do not occupy memory in the same way as variables or arrays
        // for each argument in arg_list, declare it in the symbol table and add to arg_list
        int direct_child_scope = decl_list_node->scope_id;
        symbol_entry.direct_child_scope = direct_child_scope; // Set direct child scope for function
        spdlog::info("Adding function '{}' of type '{}' with arguments to symbol table in scope {}.", this->var_name, this->data_type, scope_id);
        if (this->arg_list.has_value())
        {
            for (const auto &arg : this->arg_list.value()->arg_info)
            {
                syntax_semantic_model::SymbolEntry arg_entry;
                arg_entry.symbol_name = arg->var_name;
                arg_entry.data_type = arg->data_type;
                arg_entry.symbol_type = syntax_semantic_model::SymbolType::Variable;                     // Arguments are treated as variables
                arg_entry.scope_id = direct_child_scope;                                                 // Arguments are in the same scope as the function
                arg_entry.memory_size = (arg->data_type == "INT") ? INT_MEMORY_SIZE : FLOAT_MEMORY_SIZE; // Set memory size based on data type
                symbol_entry.arg_list->push_back(arg_entry.symbol_name);                                 // Add argument name to function's argument list
                spdlog::debug("Adding argument '{}' of type '{}' to function '{}'.", arg->var_name, arg->data_type, this->var_name);
                // check if the argument already exists in the current scope
                if (symbol_table->symbolExists(arg_entry.symbol_name, direct_child_scope))
                {
                    spdlog::error("Argument '{}' already exists in function scope {}.", arg_entry.symbol_name, direct_child_scope);
                    throw std::runtime_error("Argument already exists in function scope.");
                }
                // Add argument to symbol table
                symbol_table->addSymbol(arg_entry);
            }
        }
        else {
            // !!! INIT an empty argument list if no arguments are provided
            symbol_entry.arg_list = std::vector<std::string>();
        }
        symbol_table->addSymbol(symbol_entry);
        spdlog::info("Added function '{}' of type '{}' with arguments to symbol table in scope {}.", this->var_name, this->data_type, scope_id);
    }
};

void ast_model::TypeNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for TypeNode with scope_id: {}", scope_id);
    // Implement semantic actions for TypeNode

    // 1. set the data type for the node
    this->data_type = ast_node_type_to_string(this->subnodes[0]->node_type);
    // 2. check if the data_type is valid
    if (this->data_type != "INT" && this->data_type != "FLOAT" && this->data_type != "VOID")
    {
        spdlog::error("Invalid data type: {}", this->data_type);
        throw std::runtime_error("Invalid data type.");
    }
};

void ast_model::ArgListNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for ArgListNode with scope_id: {}", scope_id);
    // Implement semantic actions for ArgListNode
    if (subnodes.empty())
    {
        spdlog::debug("ArgListNode has no arguments.");
        this->arg_count = 0;
        return;
    }
    else
    {
        // dymanic cast subnode 0 to ArgListNode
        auto arg_list_node = std::dynamic_pointer_cast<ArgListNode>(subnodes[0]);
        // dymanic cast subnode 1 to ArgNode
        auto next_arg_node = std::dynamic_pointer_cast<ArgNode>(subnodes[1]);

        this->arg_count = arg_list_node->arg_count + 1;
        this->arg_info = arg_list_node->arg_info;
        this->arg_info.push_back(next_arg_node);
    }
};

void ast_model::ArgNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for ArgNode with scope_id: {}", scope_id);
    // Implement semantic actions for ArgNode
    // 1. dymanic cast subnode 0 to TypeNode
    auto type_node = std::dynamic_pointer_cast<TypeNode>(subnodes[0]);
    this->data_type = type_node->data_type;
    // 2. dymanic cast subnode 1 to IDNode and get var_name
    auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[1]);
    this->var_name = id_node->value;
    // 3. check if the data_type is valid
    if (this->node_type != ASTNodeType::ARG_FUNC && this->data_type == "VOID")
    {
        spdlog::error("Argument declaration cannot have VOID type.");
        throw std::runtime_error("Argument declaration cannot have VOID type.");
    }
};

void ast_model::StatListNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for StatListNode with scope_id: {}", scope_id);
    this->scope_id = scope_id; // Set the scope_id for the statement list node
    // Implement semantic actions for StatListNode
    // if subnodes are empty, return
    if (subnodes.empty())
    {
        spdlog::debug("StatListNode has no statements.");
        return;
    }
    return;
};

void ast_model::StatNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for StatNode with scope_id: {}", scope_id);
    this->scope_id = scope_id; // Set the scope_id for the statement node
    // Implement semantic actions for StatNode
    if (this->node_type == ASTNodeType::STAT_ASSIGN)
    {
        // 1. dymanic cast subnode 0 to IDNode and get var_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string var_name = id_node->value;
        // 2. check if the variable exists in the current scope
        if (!symbol_table->symbolExists(var_name, scope_id))
        {
            spdlog::error("Variable '{}' does not exist in scope {}.", var_name, scope_id);
            throw std::runtime_error("Variable does not exist in current scope.");
        }
        // 3. check if the variable is a valid type for assignment
        // dynamic cast subnode 2 to ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        // check if the data type of the expression matches the variable's data type
        auto symbol_entry = symbol_table->findSymbolInScope(var_name, scope_id);
        if (symbol_entry->data_type != expr_node->data_type)
        {
            spdlog::error("Type mismatch in assignment: variable '{}' is of type '{}', but expression is of type '{}'.", var_name, symbol_entry->data_type, expr_node->data_type);
            throw std::runtime_error("Type mismatch in assignment.");
        }
    }
    else if (this->node_type == ASTNodeType::STAT_ARRAY_ASSIGN)
    {
        // 1. dymanic cast subnode 0 to IDNode and get array_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string array_name = id_node->value;
        // 2. check if the array exists in the current scope
        if (!symbol_table->symbolExists(array_name, scope_id))
        {
            spdlog::error("Array '{}' does not exist in scope {}.", array_name, scope_id);
            throw std::runtime_error("Array does not exist in current scope.");
        }
        // 3. check if the array is a valid type for assignment
        // dynamic cast subnode 2 to ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[5]);
        // check if the data type of the expression matches the array's data type
        auto symbol_entry = symbol_table->findSymbolInScope(array_name, scope_id);
        if (symbol_entry->data_type != expr_node->data_type)
        {
            spdlog::error("Type mismatch in array assignment: array '{}' is of type '{}', but expression is of type '{}'.", array_name, symbol_entry->data_type, expr_node->data_type);
            throw std::runtime_error("Type mismatch in array assignment.");
        }
        // check if the array index is a valid integer expression
        auto index_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (index_node->data_type != "INT")
        {
            spdlog::error("Array index must be of type INT.");
            throw std::runtime_error("Array index must be of type INT.");
        }
    }
    else if (this->node_type == ASTNodeType::STAT_IF || this->node_type == ASTNodeType::STAT_IF_ELSE || this->node_type == ASTNodeType::STAT_WHILE)
    {
        ;
    }
    else if (this->node_type == ASTNodeType::STAT_RETURN)
    {
        // 1. check if the return type is valid
        if (subnodes.empty())
        {
            spdlog::error("Return statement must return a value.");
            throw std::runtime_error("Return statement must return a value.");
        }
        // 2. dymanic cast subnode 0 to ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        // 3. check if the return type matches the function's return type
        // find the direct parent scope of the current scope from scope_table
        auto parent_scope = scope_table->getDirectParentScope(scope_id);
        if (parent_scope == -1)
        {
            spdlog::error("Return statement must be inside a function.");
            throw std::runtime_error("Return statement must be inside a function.");
        }
        // find the function symbol in the symbol
        std::vector<syntax_semantic_model::SymbolEntry>
            function_symbols = symbol_table->getAllSymbolsInScope(parent_scope);
        for (const auto possible_function : function_symbols)
        {
            if (possible_function.symbol_type == syntax_semantic_model::SymbolType::Function)
            {
                // check if the return type matches the function's return type
                if (possible_function.data_type != expr_node->data_type && possible_function.direct_child_scope == scope_id)
                {
                    spdlog::error("Return type mismatch: function '{}' expects return type '{}', but got '{}'.", possible_function.symbol_name, possible_function.data_type, expr_node->data_type);
                    throw std::runtime_error("Return type mismatch.");
                }
                return; // found the function, no need to check further
            }
        }
    }
    else if (this->node_type == ASTNodeType::STAT_COMPOUND)
    {
        ; // pass, compound statements do not require special handling
    }
    else if (this->node_type == ASTNodeType::STAT_FUNC_CALL)
    {
        // 1. dymanic cast subnode 0 to IDNode and get function_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string function_name = id_node->value;
        // 2. check if the function exists in the current scope
        if (!symbol_table->symbolExists(function_name, scope_id))
        {
            spdlog::error("Function '{}' does not exist in scope {}.", function_name, scope_id);
            throw std::runtime_error("Function does not exist in current scope.");
        }
        // 3. check if the function is called with the correct number of arguments
        auto symbol_entry = symbol_table->findSymbolInScope(function_name, scope_id);
        auto rarg_list_subnode = std::dynamic_pointer_cast<RealArgListNode>(subnodes[2]);
        if (rarg_list_subnode->arg_count != symbol_entry->arg_list->size())
        {
            spdlog::error("Function '{}' expects {} arguments, but got {}.", function_name, symbol_entry->arg_list->size(), rarg_list_subnode->arg_count);
            throw std::runtime_error("Function call argument count mismatch.");
        }
        // 4. check if the argument types match
        for (size_t i = 0; i < rarg_list_subnode->arg_count; ++i)
        {
            auto rarg_node = rarg_list_subnode->arg_info[i];
            if (!symbol_entry->direct_child_scope.has_value())
            {
                spdlog::error("Function '{}' does not have a direct child scope.", function_name);
                throw std::runtime_error("Function does not have a direct child scope.");
            }
            int direct_child_scope = symbol_entry->direct_child_scope.value();
            auto arg_node = symbol_table->findSymbolInScope(symbol_entry->arg_list->at(i), direct_child_scope);
            if (arg_node->data_type != rarg_node->data_type)
            {
                spdlog::error("Argument type mismatch for function '{}': expected '{}', but got '{}'.", function_name, arg_node->data_type, rarg_node->data_type);
                throw std::runtime_error("Function call argument type mismatch.");
            }
        }
    }
    else if (this->node_type == ASTNodeType::STAT_IFELSE_MS || this->node_type == ASTNodeType::STAT_IFELSE_UMS || this->node_type == ASTNodeType::STAT_IF_ELSE_CHAIN)
    {
        ;
    }
};

void ast_model::ExprNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for ExprNode with scope_id: {}", scope_id);
    // Implement semantic actions for ExprNode
    // 1. check if the expression is a valid variable or array access
    if (subnodes.empty())
    {
        spdlog::error("Expression node has no subnodes.");
        throw std::runtime_error("Expression node has no subnodes.");
    }
    // cases
    if (this->node_type == ASTNodeType::EXPR_CONST)
    {
        if (subnodes[0]->node_type == ASTNodeType::NUM)
        {
            // if the subnode is a number, set the data type to INT
            this->data_type = "INT";
            this->value = std::dynamic_pointer_cast<TerminalNode>(subnodes[0])->value;
        }
        else if (subnodes[0]->node_type == ASTNodeType::FLO)
        {
            // if the subnode is a float, set the data type to FLOAT
            this->data_type = "FLOAT";
            this->value = std::dynamic_pointer_cast<TerminalNode>(subnodes[0])->value;
        }
        else
        {
            spdlog::error("Invalid constant expression.");
            throw std::runtime_error("Invalid constant expression.");
        }
    }
    else if (this->node_type == ASTNodeType::EXPR_VAR)
    {
        // 2. dymanic cast subnode 0 to IDNode and get var_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string var_name = id_node->value;
        // 3. check if the variable exists in the current scope
        if (!symbol_table->symbolExists(var_name, scope_id))
        {
            spdlog::error("Variable '{}' does not exist in scope {}.", var_name, scope_id);
            throw std::runtime_error("Variable does not exist in current scope.");
        }
        // 4. get the variable's data type from the symbol table
        auto symbol_entry = symbol_table->findSymbolInScope(var_name, scope_id);
        this->data_type = symbol_entry->data_type;
        this->value = symbol_entry->symbol_name; // Set value to the variable name for reference
    }
    else if (this->node_type == ASTNodeType::EXPR_ARRAY)
    {
        // 5. dymanic cast subnode 0 to IDNode and get array_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string array_name = id_node->value;
        // 6. check if the array exists in the current scope
        if (!symbol_table->symbolExists(array_name, scope_id))
        {
            spdlog::error("Array '{}' does not exist in scope {}.", array_name, scope_id);
            throw std::runtime_error("Array does not exist in current scope.");
        }
        // 7. check if the array index is a valid integer expression
        auto index_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (index_node->data_type != "INT")
        {
            spdlog::error("Array index must be of type INT.");
            throw std::runtime_error("Array index must be of type INT.");
        }
        // 8. get the array's data type from the symbol table
        auto symbol_entry = symbol_table->findSymbolInScope(array_name, scope_id);
        this->data_type = symbol_entry->data_type;
    }
    else if (this->node_type == ASTNodeType::EXPR_FUNC)
    {
        // the same as STAT_FUNC_CALL, but this is an expression node, if successful, set the data type to the function's return type
        // 1. dymanic cast subnode 0 to IDNode and get function_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string function_name = id_node->value;
        // 2. check if the function exists in the current scope
        if (!symbol_table->symbolExists(function_name, scope_id))
        {
            spdlog::error("Function '{}' does not exist in scope {}.", function_name, scope_id);
            throw std::runtime_error("Function does not exist in current scope.");
        }
        // 3. check if the function is called with the correct number of arguments
        auto symbol_entry = symbol_table->findSymbolInScope(function_name, scope_id);
        auto rarg_list_subnode = std::dynamic_pointer_cast<RealArgListNode>(subnodes[2]);
        if (rarg_list_subnode->arg_count != symbol_entry->arg_list->size())
        {
            spdlog::error("Function '{}' expects {} arguments, but got {}.", function_name, symbol_entry->arg_list->size(), rarg_list_subnode->arg_count);
            throw std::runtime_error("Function call argument count mismatch.");
        }
        // 4. check if the argument types match
        for (size_t i = 0; i < rarg_list_subnode->arg_count; ++i)
        {
            auto rarg_node = rarg_list_subnode->arg_info[i];
            if (!symbol_entry->direct_child_scope.has_value())
            {
                spdlog::error("Function '{}' does not have a direct child scope.", function_name);
                throw std::runtime_error("Function does not have a direct child scope.");
            }
            int direct_child_scope = symbol_entry->direct_child_scope.value();
            auto arg_node = symbol_table->findSymbolInScope(symbol_entry->arg_list->at(i), direct_child_scope);
            if (arg_node->data_type != rarg_node->data_type)
            {
                spdlog::error("Argument type mismatch for function '{}': expected '{}', but got '{}'.", function_name, arg_node->data_type, rarg_node->data_type);
                throw std::runtime_error("Function call argument type mismatch.");
            }
        }
        // 5. set the data type to the function's return type
        this->data_type = symbol_entry->data_type;
    }
    else if (this->node_type == ASTNodeType::EXPR_ARITH_NOCONST)
    {
        // 9. check if the expression is a valid arithmetic expression
        if (subnodes.size() < 3)
        {
            spdlog::error("Arithmetic expression must have at least two operands and one operator.");
            throw std::runtime_error("Invalid arithmetic expression.");
        }
        // 10. check if the operands are of valid types
        auto left_operand = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        auto right_operand = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (left_operand->data_type != right_operand->data_type)
        {
            spdlog::error("Type mismatch in arithmetic expression: left operand is of type '{}', but right operand is of type '{}'.", left_operand->data_type, right_operand->data_type);
            throw std::runtime_error("Type mismatch in arithmetic expression.");
        }
        // 11. set the data type to the operand's data type
        this->data_type = left_operand->data_type;
        // 12. check if both subnodes are EXPR_CONST
        if (left_operand->node_type == ASTNodeType::EXPR_CONST && right_operand->node_type == ASTNodeType::EXPR_CONST)
        {
            // If both operands are constants, evaluate the expression
            if (this->subnodes[1]->node_type == ASTNodeType::ADD)
            {
                // If the operator is ADD, add the two constants
                if (this->data_type == "INT")
                {
                    this->value = std::to_string(std::stoi(left_operand->value.value()) + std::stoi(right_operand->value.value()));
                }
                else if (this->data_type == "FLOAT")
                {
                    this->value = std::to_string(std::stod(left_operand->value.value()) + std::stod(right_operand->value.value()));
                }
                else
                {
                    spdlog::error("Invalid data type for addition: {}", this->data_type);
                    throw std::runtime_error("Invalid data type for addition.");
                }
            }
            else if (this->subnodes[1]->node_type == ASTNodeType::MUL)
            {
                // If the operator is MUL, multiply the two constants
                this->value = std::to_string(std::stod(left_operand->value.value()) * std::stod(right_operand->value.value()));
            }
            else
            {
                spdlog::error("Invalid arithmetic operator in expression.");
                throw std::runtime_error("Invalid arithmetic operator.");
            }
            this->node_type = ASTNodeType::EXPR_CONST; // Change node type to EXPR_CONST since we have evaluated it
        }
        else
        {
            // If not both constants, just set value to empty string
            this->value = "";
        }
    }
    else if (this->node_type == ASTNodeType::EXPR_PAREN_NOCONST)
    {
        // 12. check if the expression is a valid parenthesized expression
        if (subnodes.size() != 1)
        {
            spdlog::error("Parenthesized expression must have exactly one subnode.");
            throw std::runtime_error("Invalid parenthesized expression.");
        }
        // 13. set the data type to the subnode's data type
        auto sub_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        this->data_type = sub_expr_node->data_type;
        this->value = sub_expr_node->value;
        // If the subnode is a constant, set the value to the subnode's value
        if (sub_expr_node->node_type == ASTNodeType::EXPR_CONST)
        {
            this->node_type = ASTNodeType::EXPR_CONST; // Change node type to EXPR_CONST since we have evaluated it
            this->value = sub_expr_node->value;        // Set value to the subnode's value
        }
    }
    else if (this->node_type == ASTNodeType::EXPR_MUL_TEMP || this->node_type == ASTNodeType::EXPR_ATOMIC_TEMP)
    {
        // CASE 1: E -> E', E' is a temporary node for solving ambiguity in multiplication
        // CASE 2: E' -> E_ATOMIC, E_ATOMIC is a temporary node for enforcing the left associativity of multiplication
        if (subnodes.size() != 1)
        {
            spdlog::error("Temporary multiplication expression must have exactly one subnode.");
            throw std::runtime_error("Invalid temporary multiplication expression.");
        }
        // 14. set the data type to the subnode's data type
        auto sub_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        this->data_type = sub_expr_node->data_type;
        // if the subnode is a constant, set the value to the subnode's value and update the node type
        if (sub_expr_node->node_type == ASTNodeType::EXPR_CONST)
        {
            this->node_type = ASTNodeType::EXPR_CONST; // Change node type to EXPR_CONST since we have evaluated it
            this->value = sub_expr_node->value;        // Set value to the subnode's value
        }
        else
        {
            this->value = ""; // If not a constant, set value to empty string
        }
    }
    else
    {
        spdlog::error("Invalid expression node type: {}", ast_node_type_to_string(this->node_type));
        throw std::runtime_error("Invalid expression node type.");
    }
};

void ast_model::BoolNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for BoolNode with scope_id: {}", scope_id);
    // Implement semantic actions for BoolNode
    // 1. check if the boolean expression is valid
    if (subnodes.empty())
    {
        spdlog::error("Boolean expression node has no subnodes.");
        throw std::runtime_error("Boolean expression node has no subnodes.");
    }
    // 2. cases
    if (this->node_type == ASTNodeType::BOOL_EXPR)
    {
        // 3. dymanic cast subnode 0 to ExprNode and get data type
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        if (expr_node->data_type != "INT")
        {
            spdlog::error("Boolean expression must be of type INT.");
            throw std::runtime_error("Boolean expression must be of type INT.");
        }
    }
    else if (this->node_type == ASTNodeType::BOOL_OP)
    {
        // 5. check if the boolean operation is valid
        if (subnodes.size() < 3)
        {
            spdlog::error("Boolean operation must have at least two operands and one operator.");
            throw std::runtime_error("Invalid boolean operation.");
        }
        // 6. check if the operands are of valid types
        auto left_operand = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        auto right_operand = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (left_operand->data_type != "INT" || right_operand->data_type != "INT")
        {
            spdlog::error("Both operands of boolean operation must be of type INT.");
            throw std::runtime_error("Both operands of boolean operation must be of type INT.");
        }
    }
    else
    {
        spdlog::error("Invalid boolean node type: {}", ast_node_type_to_string(this->node_type));
        throw std::runtime_error("Invalid boolean node type.");
    }
};

void ast_model::RealArgListNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for RealArgListNode with scope_id: {}", scope_id);
    // Implement semantic actions for RealArgListNode
    if (subnodes.empty())
    {
        spdlog::debug("RealArgListNode has no arguments.");
        this->arg_count = 0;
        return;
    }
    else
    {
        // dymanic cast subnode 0 to RealArgListNode
        auto real_arg_list_node = std::dynamic_pointer_cast<RealArgListNode>(subnodes[0]);
        // dymanic cast subnode 1 to RealArgNode
        auto next_real_arg_node = std::dynamic_pointer_cast<RealArgNode>(subnodes[1]);

        this->arg_count = real_arg_list_node->arg_count + 1;
        this->arg_info = real_arg_list_node->arg_info;
        this->arg_info.push_back(next_real_arg_node);
    }
};

void ast_model::RealArgNode::semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table)
{
    spdlog::info("Performing semantic action for RealArgNode with scope_id: {}", scope_id);
    // Implement semantic actions for RealArgNode
    // cases
    if (this->node_type == ASTNodeType::RARG_EXPR)
    {
        // 1. dymanic cast subnode 0 to ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        this->data_type = expr_node->data_type; // Set data type to the expression's data type
        this->value = expr_node->value;         // Set value to the expression's value
    }
    else if (this->node_type == ASTNodeType::RARG_ARRAY)
    {
        // 2. dymanic cast subnode 0 to IDNode and get var_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string array_name = id_node->value;
        // 3. check if the variable exists in the current scope
        if (!symbol_table->symbolExists(array_name, scope_id))
        {
            spdlog::error("Variable '{}' does not exist in scope {}.", array_name, scope_id);
            throw std::runtime_error("Variable does not exist in current scope.");
        }
        // 4. get the variable's data type from the symbol table
        auto symbol_entry = symbol_table->findSymbolInScope(array_name, scope_id);
        this->data_type = symbol_entry->data_type;
    }
    else if (this->node_type == ASTNodeType::RARG_FUNC)
    {
        // 5. dymanic cast subnode 0 to IDNode and get function_name
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string function_name = id_node->value;
        // 6. check if the function exists in the current scope
        if (!symbol_table->symbolExists(function_name, scope_id))
        {
            spdlog::error("Function '{}' does not exist in scope {}.", function_name, scope_id);
            throw std::runtime_error("Function does not exist in current scope.");
        }
        // 7. get the function's return type from the symbol table
        auto symbol_entry = symbol_table->findSymbolInScope(function_name, scope_id);
        this->data_type = symbol_entry->data_type;
    }
    else
    {
        spdlog::error("Invalid real argument node type: {}", ast_node_type_to_string(this->node_type));
        throw std::runtime_error("Invalid real argument node type.");
    }
};

// visualization functions
std::string ast_model::ProgramNode::to_string() const
{
    return "Program";
}

std::string ast_model::DeclListNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(count=" + std::to_string(decl_count) + ", scope=" + std::to_string(scope_id) + ")";
}

std::string ast_model::DeclNode::to_string() const
{
    std::string s = ast_node_type_to_string(node_type) + "(type=" + data_type + ", name=" + var_name;
    if (array_length.has_value())
    {
        s += ", arrlen=" + std::to_string(array_length.value());
    }
    if (arg_list.has_value())
    {
        s += ", argc=" + std::to_string(arg_list.value()->arg_count);
        if (function_sub_scope.has_value())
            s += ", fscope=" + std::to_string(function_sub_scope.value());
    }
    s += ", scope=" + std::to_string(scope_id) + ")";
    return s;
}

std::string ast_model::TypeNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(" + data_type + ")";
}

std::string ast_model::ArgListNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(count=" + std::to_string(arg_count) + ")";
}

std::string ast_model::ArgNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(type=" + data_type + ", name=" + var_name + ")";
}

std::string ast_model::StatListNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(scope=" + std::to_string(scope_id) + ")";
}

std::string ast_model::StatNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(scope=" + std::to_string(scope_id) + ")";
}

std::string ast_model::ExprNode::to_string() const
{
    std::string s = ast_node_type_to_string(node_type) + "(type=" + data_type;
    if (value.has_value() && value.value() != "")
        s += ", val=" + value.value();
    s += ")";
    return s;
}

std::string ast_model::BoolNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(scope=" + std::to_string(scope_id) + ")";
}

std::string ast_model::RealArgNode::to_string() const
{
    std::string s = ast_node_type_to_string(node_type) + "(type=" + data_type;
    if (value.has_value() && value.value() != "")
        s += ", val=" + value.value();
    s += ")";
    return s;
}

std::string ast_model::RealArgListNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(count=" + std::to_string(arg_count) + ", scope=" + std::to_string(scope_id) + ")";
}

std::string ast_model::TerminalNode::to_string() const
{
    return ast_node_type_to_string(node_type) + "(type=" + ast_node_type_to_string(node_type) + ", val=" + value + ")";
}