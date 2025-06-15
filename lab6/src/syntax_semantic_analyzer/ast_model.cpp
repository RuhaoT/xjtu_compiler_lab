#include "ast_model.h"
#include "symbol_table.h"
#include "interm_code/interm_code_model.h"
#include "logical_env_simulator.h"
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