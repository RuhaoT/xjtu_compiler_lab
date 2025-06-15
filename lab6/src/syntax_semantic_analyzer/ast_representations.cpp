#include "ast_model.h"
#include "spdlog/spdlog.h"

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