#include "ast_model.h"
#include "symbol_table.h"
#include "interm_code/interm_code_model.h"
#include "logical_env_simulator.h"
#include "scope_table.h"
#include "spdlog/spdlog.h"

// interm code generation function
void ast_model::ProgramNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for ProgramNode.");
    // Implement intermediate code generation for ProgramNode
    // dynamic cast subnode 0 to DeclListNode and subnode 1 to StatListNode
    auto decl_list_node = std::dynamic_pointer_cast<DeclListNode>(subnodes[0]);
    auto stat_list_node = std::dynamic_pointer_cast<StatListNode>(subnodes[1]);
    if (!decl_list_node || !stat_list_node)
    {
        spdlog::error("ProgramNode must have a DeclListNode and a StatListNode as subnodes.");
        throw std::runtime_error("Invalid ProgramNode structure.");
    }
    if (!decl_list_node->interm_code_list.has_value() || !stat_list_node->interm_code_list.has_value())
    {
        spdlog::error("DeclListNode or StatListNode does not have intermediate code list.");
        throw std::runtime_error("Intermediate code list is not available.");
    }
    // add existing intermcodes
    simulator.add_intermediate_code_batch(
        decl_list_node->interm_code_list.value());
    simulator.add_intermediate_code_batch(
        stat_list_node->interm_code_list.value());
    // code generation complete (final return statement is handled in StatListNode)
    spdlog::info("Intermediate code generation for ProgramNode completed.");
}

void ast_model::DeclListNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for DeclListNode.");
    // Implement intermediate code generation for DeclListNode
    // 1. check if the decl_count is 0, if so, init the intermediate code list and return
    if (decl_count == 0)
    {
        spdlog::debug("DeclListNode has no declarations, this is an initialization step.");
        this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
        return;
    }
    else
    {
        // cast subnode 0 to DeclListNode & subnode 1 to DeclNode
        auto decl_list_node = std::dynamic_pointer_cast<DeclListNode>(subnodes[0]);
        auto next_decl_node = std::dynamic_pointer_cast<DeclNode>(subnodes[1]);
        if (!decl_list_node || !next_decl_node)
        {
            spdlog::error("DeclListNode must have a DeclListNode and a DeclNode as subnodes.");
            throw std::runtime_error("Invalid DeclListNode structure.");
        }
        // add the intermediate code of the subnode 0 to the current node's intermediate code list
        if (decl_list_node->interm_code_list.has_value())
        {
            this->interm_code_list = decl_list_node->interm_code_list.value();
        }
        else
        {
            std::string err_msg = "DeclListNode does not have an initialized intermediate code list.";
            spdlog::error(err_msg);
            throw std::runtime_error(err_msg);
        }
        // append the intermediate code of the next declaration node
        if (next_decl_node->interm_code_list.has_value())
        {
            auto next_interm_code_list = next_decl_node->interm_code_list.value();
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                next_interm_code_list.begin(),
                next_interm_code_list.end());
            spdlog::debug("Added {} intermediate codes from DeclNode to DeclListNode.", next_interm_code_list.size());
        }
        else
        {
            std::string err_msg = "DeclNode does not have an initialized intermediate code list.";
            spdlog::error(err_msg);
            throw std::runtime_error(err_msg);
        }
    }
};

void ast_model::DeclNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for DeclNode.");
    // Implement intermediate code generation for DeclNode
    // 0. init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // 1. if this is a variable declaration, no need to generate intermediate code(already done in semantic action)
    if (this->node_type == ASTNodeType::DECL_VAR)
    {
        spdlog::debug("DeclNode is a variable declaration, no intermediate code generated.");
        return;
    }
    else if (this->node_type == ASTNodeType::DECL_ARRAY)
    {
        // 2. for array, first check length, then reserve space in the memory and document initial address, save the address to a new register
        if (!this->array_length.has_value())
        {
            spdlog::error("Array declaration must have a length.");
            throw std::runtime_error("Array declaration must have a length.");
        }
        int mem_unit_count = this->array_length.value();
        if (this->data_type == ast_node_type_to_string(ASTNodeType::INT))
        {
            // for INT, each unit is 4 bytes
            mem_unit_count *= 4;
        }
        else if (this->data_type == ast_node_type_to_string(ASTNodeType::FLOAT))
        {
            // for FLOAT, each unit is 8 bytes
            mem_unit_count *= 8;
        }
        else
        {
            spdlog::error("Invalid data type for array declaration: {}", this->data_type);
            throw std::runtime_error("Invalid data type for array declaration.");
        }
        // get the current free memory address
        int current_free_address = simulator.get_memory_usage();
        // reserve memory for the array
        simulator.allocate_memory(mem_unit_count);
        // store the address in a new register
        interm_code_model::Register arr_initial_addr_reg = simulator.get_new_treg(scope_id);
        // generate the intermediate code for storing the initial address of the array
        interm_code_model::LogicalAddress arr_initial_addr(
            interm_code_model::LogicalMemSpaceType::DATA,
            current_free_address);
        auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            arr_initial_addr_reg,
            interm_code_model::Operand(arr_initial_addr));
        // register the array to arr-reg map
        simulator.register_arr(this->var_name, scope_id, arr_initial_addr_reg);
        // add the intermediate code to the list
        this->interm_code_list->push_back(interm_code);
        spdlog::debug("Generated intermediate code for array declaration: {} with initial address register {}", this->var_name, arr_initial_addr_reg.toString());
    }
    else if (this->node_type == ASTNodeType::DECL_FUNC)
    {
        // 3. function
        spdlog::debug("DeclNode is a function declaration, generating intermediate code.");
        // 1. get label
        std::string func_label = simulator.get_func_label(this->var_name);
        spdlog::debug("Function label for {} is {}", this->var_name, func_label);
        // 2. generate header code for the function
        if (!this->arg_list.has_value())
        {
            spdlog::error("Function declaration must have an argument list.");
            throw std::runtime_error("Function declaration must have an argument list.");
        }
        auto header_code = simulator.generate_func_header(this->arg_list.value()->arg_count, func_label);
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            header_code.begin(),
            header_code.end());
        spdlog::debug("Generated function header code for {} with {} arguments.", this->var_name, this->arg_list.value()->arg_count);
        // 3. cast subnode 6 to decllist node and 7 to statlist node
        auto decl_list_node = std::dynamic_pointer_cast<DeclListNode>(subnodes[6]);
        auto stat_list_node = std::dynamic_pointer_cast<StatListNode>(subnodes[7]);
        if (!decl_list_node || !stat_list_node)
        {
            spdlog::error("Function declaration must have a DeclListNode and a StatListNode as subnodes.");
            throw std::runtime_error("Invalid function declaration structure.");
        }
        // append the intermediate code of the decl list node
        if (decl_list_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                decl_list_node->interm_code_list.value().begin(),
                decl_list_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from DeclListNode to DeclNode.", decl_list_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("DeclListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("DeclListNode does not have an initialized intermediate code list.");
        }
        // append the intermediate code of the stat list node
        if (stat_list_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                stat_list_node->interm_code_list.value().begin(),
                stat_list_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from StatListNode to DeclNode.", stat_list_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("StatListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("StatListNode does not have an initialized intermediate code list.");
        }
        // 4. check if there is a return statement in the function
        bool has_return = false;
        for (const auto &stat_node : stat_list_node->subnodes)
        {
            if (stat_node->node_type == ASTNodeType::STAT_RETURN)
            {
                has_return = true;
                break;
            }
        }
        if (!has_return)
        {
            throw std::runtime_error("Function " + this->var_name + " must have a return statement.");
        }
    }
    else
    {
        // unknown node type
        spdlog::error("Invalid declaration node type: {}", ast_node_type_to_string(this->node_type));
        throw std::runtime_error("Invalid declaration node type.");
    }
}

void ast_model::TypeNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("TypeNode does not generate intermediate code, it is only used for semantic analysis.");
};

void ast_model::ArgListNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    // pass
    spdlog::info("ArgListNode does not generate intermediate code, it is only used for semantic analysis.");
};

void ast_model::ArgNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    // pass
    spdlog::info("ArgNode does not generate intermediate code, it is only used for semantic analysis.");
};

void ast_model::StatListNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for StatListNode.");
    // Implement intermediate code generation for StatListNode
    // 1. check subnode count
    int subnode_count = subnodes.size();
    // if subnode count is 1, init the intermediate code list and return
    if (subnode_count == 1)
    {
        spdlog::debug("StatListNode has 1 subnode-StatNode, this is an initialization step.");
        auto stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[0]);
        this->interm_code_list = stat_node->interm_code_list;
        if (!this->interm_code_list.has_value())
        {
            spdlog::error("StatNode does not have an initialized intermediate code list.");
            throw std::runtime_error("StatNode does not have an initialized intermediate code list.");
        }
        spdlog::info("Intermediate code generation for StatListNode completed with 1 subnode.");
        return;
    }
    // 2. cast subnode 0 to StatListNode and subnode 2 to StatNode
    auto stat_list_node = std::dynamic_pointer_cast<StatListNode>(subnodes[0]);
    auto next_stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[2]);
    if (!stat_list_node || !next_stat_node)
    {
        spdlog::error("StatListNode must have a StatListNode and a StatNode as subnodes.");
        throw std::runtime_error("Invalid StatListNode structure.");
    }
    // 3. add the intermediate code of the subnode 0 to the current node's intermediate code list
    if (stat_list_node->interm_code_list.has_value())
    {
        this->interm_code_list = stat_list_node->interm_code_list.value();
    }
    else
    {
        std::string err_msg = "StatListNode does not have an initialized intermediate code list.";
        spdlog::error(err_msg);
        throw std::runtime_error(err_msg);
    }
    // 4. append the intermediate code of the next statement node
    if (next_stat_node->interm_code_list.has_value())
    {
        auto next_interm_code_list = next_stat_node->interm_code_list.value();
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            next_interm_code_list.begin(),
            next_interm_code_list.end());
        spdlog::debug("Added {} intermediate codes from StatNode to StatListNode.", next_interm_code_list.size());
    }
    else
    {
        std::string err_msg = "StatNode does not have an initialized intermediate code list.";
        spdlog::error(err_msg);
        throw std::runtime_error(err_msg);
    }
    // return
    spdlog::info("Intermediate code generation for StatListNode completed.");
};

void ast_model::StatNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for StatNode.");
    // init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // Implement intermediate code generation for StatNode
    if (node_type == ast_model::ASTNodeType::STAT_ASSIGN)
    {
        // cast 0 to ID and 2 to ExprNode
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (!id_node || !expr_node)
        {
            spdlog::error("STAT_ASSIGN must have an IDNode and an ExprNode as subnodes.");
            throw std::runtime_error("Invalid STAT_ASSIGN structure.");
        }
        // get variable register(this will check if the variable exists in the current scope)
        interm_code_model::Register var_reg = simulator.get_var_reg(id_node->value, scope_id);
        // get the expression's result register
        if (!expr_node->result_register.has_value())
        {
            spdlog::error("Expression node does not have a result register.");
            throw std::runtime_error("Expression node does not have a result register.");
        }
        interm_code_model::Register expr_reg = expr_node->result_register.value();
        // take in the intermediate code of the expression node
        if (expr_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                expr_node->interm_code_list.value().begin(),
                expr_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from ExprNode to StatNode.", expr_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("ExprNode does not have an initialized intermediate code list.");
            throw std::runtime_error("ExprNode does not have an initialized intermediate code list.");
        }
        // generate the intermediate code for assignment
        auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            var_reg,
            interm_code_model::Operand(expr_reg));
        // add the intermediate code to the list
        this->interm_code_list->push_back(interm_code);
        spdlog::debug("Generated intermediate code for assignment: {} = {}", id_node->value, expr_node->value.value_or("N/A"));
    }
    else if (node_type == ast_model::ASTNodeType::STAT_ARRAY_ASSIGN)
    {
        // cast 0 to ID and 2 to ExprNode, 5 to ExprNode
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        auto arr_index_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[5]);
        if (!id_node || !arr_index_node || !expr_node)
        {
            spdlog::error("STAT_ARRAY_ASSIGN must have an IDNode, an ExprNode for index, and an ExprNode for value as subnodes.");
            throw std::runtime_error("Invalid STAT_ARRAY_ASSIGN structure.");
        }
        // get array init addr register(this will check if the array exists in the current scope)
        interm_code_model::Register arr_reg = simulator.get_arr_reg(id_node->value, scope_id);
        // calculate the address of the array element
        // 1. new_reg = index * element_size
        int element_size = expr_node->data_type == ast_node_type_to_string(ASTNodeType::INT) ? 4 : 8; // INT is 4 unit, FLOAT is 8 unit
        interm_code_model::Register new_reg = simulator.get_new_treg(scope_id);
        auto code_mul = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::MUL,
            interm_code_model::Operand(new_reg),
            interm_code_model::Operand(arr_index_node->result_register.value()),
            interm_code_model::Operand(std::to_string(element_size), interm_code_model::OperandType::CONSTANT));
        this->interm_code_list->push_back(code_mul);
        // 2. new_reg = arr_reg + new_reg
        // can reuse this new_reg
        auto code_add = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ADD,
            interm_code_model::Operand(new_reg),
            interm_code_model::Operand(arr_reg),
            interm_code_model::Operand(new_reg));
        this->interm_code_list->push_back(code_add);
        // 3. store the expression value to the calculated address
        // store expr_node result reg to the calculated address(new_reg)
        if (!expr_node->result_register.has_value())
        {
            spdlog::error("Expression node does not have a result register.");
            throw std::runtime_error("Expression node does not have a result register.");
        }
        interm_code_model::Register expr_reg = expr_node->result_register.value();
        auto code_store = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::STORE,
            interm_code_model::Operand(new_reg),
            interm_code_model::Operand(expr_reg));
        this->interm_code_list->push_back(code_store);
        spdlog::debug("Generated intermediate code for array assignment: {}[{}] = {}", id_node->value, arr_index_node->value.value_or("N/A"), expr_node->value.value_or("N/A"));
    }
    else if (node_type == ast_model::ASTNodeType::STAT_IF)
    {
        // cast 2 to BoolNode and 4 to StatListNode
        auto bool_node = std::dynamic_pointer_cast<BoolNode>(subnodes[2]);
        auto stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[4]);
        if (!bool_node || !stat_node)
        {
            spdlog::error("STAT_IF must have a BoolNode and a StatListNode as subnodes.");
            throw std::runtime_error("Invalid STAT_IF structure.");
        }
        // 1. add the intermediate code for the boolean expression to the list
        if (!bool_node->result_register.has_value())
        {
            spdlog::error("BoolNode does not have a result register.");
            throw std::runtime_error("BoolNode does not have a result register.");
        }
        interm_code_model::Register bool_reg = bool_node->result_register.value();
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            bool_node->interm_code_list.value().begin(),
            bool_node->interm_code_list.value().end());
        spdlog::debug("Added {} intermediate codes from BoolNode to StatNode.", bool_node->interm_code_list.value().size());
        // 2. get 2 new labels: one for the true branch and one for the end of the if statement
        std::string true_label = simulator.get_new_temp_label(scope_id);
        std::string end_label = simulator.get_new_temp_label(scope_id);
        // 3. generate a goto_if code, destination is the true label, criteria is the boolean expression result register
        auto goto_if_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO_IF,
            interm_code_model::Operand(bool_reg),
            interm_code_model::Operand(true_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(goto_if_code);
        spdlog::debug("Generated GOTO_IF code for STAT_IF with label: {}", true_label);
        // 4. generate a goto code to the end label(this is to skip the false branch)
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            interm_code_model::Operand(end_label, interm_code_model::OperandType::LABEL),
            std::nullopt,
            std::nullopt);
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to end label: {}", end_label);
        // 5. add an empty intermediate code for the true branch, label it with the true label
        auto true_branch_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            true_label);
        this->interm_code_list->push_back(true_branch_code);
        spdlog::debug("Added empty intermediate code for true branch with label: {}", true_label);
        // 6. add the intermediate code of the stat list node to the list
        if (stat_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                stat_node->interm_code_list.value().begin(),
                stat_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from StatListNode to StatNode.", stat_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("StatListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("StatListNode does not have an initialized intermediate code list.");
        }
        // 7. add an empty intermediate code for the end of the if statement, label it with the end label
        auto end_branch_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            end_label);
        this->interm_code_list->push_back(end_branch_code);
        spdlog::debug("Added empty intermediate code for end of STAT_IF with label: {}", end_label);
    }
    else if (node_type == ast_model::ASTNodeType::STAT_IF_ELSE)
    {
        // pretty much the same as STAT_IF, but with an else branch(an extra label and goto code)
        // cast 2 to BoolNode, 4 to StatListNode for true branch, and 6 to StatListNode for false branch
        auto bool_node = std::dynamic_pointer_cast<BoolNode>(subnodes[2]);
        auto true_stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[4]);
        auto false_stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[6]);
        if (!bool_node || !true_stat_node || !false_stat_node)
        {
            spdlog::error("STAT_IF_ELSE must have a BoolNode, a StatNode for true branch, and a StatNode for false branch as subnodes.");
            throw std::runtime_error("Invalid STAT_IF_ELSE structure.");
        }
        // 1. add the intermediate code for the boolean expression to the list
        if (!bool_node->result_register.has_value())
        {
            spdlog::error("BoolNode does not have a result register.");
            throw std::runtime_error("BoolNode does not have a result register.");
        }
        interm_code_model::Register bool_reg = bool_node->result_register.value();
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            bool_node->interm_code_list.value().begin(),
            bool_node->interm_code_list.value().end());
        spdlog::debug("Added {} intermediate codes from BoolNode to StatNode.", bool_node->interm_code_list.value().size());
        // 2. get 3 new labels: one for the true branch, one for the false branch, and one for the end of the if statement
        std::string true_label = simulator.get_new_temp_label(scope_id);
        std::string false_label = simulator.get_new_temp_label(scope_id);
        std::string end_label = simulator.get_new_temp_label(scope_id);
        // 3. generate a goto_if code, destination is the true label, criteria is the boolean expression result register
        auto goto_if_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO_IF,
            interm_code_model::Operand(bool_reg),
            interm_code_model::Operand(true_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(goto_if_code);
        spdlog::debug("Generated GOTO_IF code for STAT_IF_ELSE with label: {}", true_label);
        // 4. generate a goto code to the false label(this is to skip the true branch)
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            interm_code_model::Operand(false_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to false label: {}", false_label);
        // 5. add an empty intermediate code for the true branch, label it with the true label
        auto true_branch_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            true_label);
        this->interm_code_list->push_back(true_branch_code);
        spdlog::debug("Added empty intermediate code for true branch with label: {}", true_label);
        // 6. add the intermediate code of the true stat node to the list
        if (true_stat_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                true_stat_node->interm_code_list.value().begin(),
                true_stat_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from true StatNode to StatNode.", true_stat_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("True StatNode does not have an initialized intermediate code list.");
            throw std::runtime_error("True StatNode does not have an initialized intermediate code list.");
        }
        // 7. !!! generate a goto code to the end label(this is to skip the false branch)
        auto goto_end_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            interm_code_model::Operand(end_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(goto_end_code);
        spdlog::debug("Generated GOTO code to end label: {}", end_label);
        // 8. add an empty intermediate code for the false branch, label it with the false label
        auto false_branch_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            false_label);
        this->interm_code_list->push_back(false_branch_code);
        spdlog::debug("Added empty intermediate code for false branch with label: {}", false_label);
        // 9. add the intermediate code of the false stat node to the list
        if (false_stat_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                false_stat_node->interm_code_list.value().begin(),
                false_stat_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from false StatNode to StatNode.", false_stat_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("False StatNode does not have an initialized intermediate code list.");
            throw std::runtime_error("False StatNode does not have an initialized intermediate code list.");
        }
        // 10. add an empty intermediate code for the end of the if statement, label it with the end label
        auto end_branch_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            end_label);
        this->interm_code_list->push_back(end_branch_code);
        spdlog::debug("Added empty intermediate code for end of STAT_IF_ELSE with label: {}", end_label);
    }
    else if (node_type == ast_model::ASTNodeType::STAT_WHILE)
    {
        // cast 2 to BoolNode and 4 to StatListNode
        auto bool_node = std::dynamic_pointer_cast<BoolNode>(subnodes[2]);
        auto stat_node = std::dynamic_pointer_cast<StatNode>(subnodes[4]);
        if (!bool_node || !stat_node)
        {
            spdlog::error("STAT_WHILE must have a BoolNode and a StatNode as subnodes.");
            throw std::runtime_error("Invalid STAT_WHILE structure.");
        }
        // 1. add the intermediate code for the boolean expression to the list
        if (!bool_node->result_register.has_value())
        {
            spdlog::error("BoolNode does not have a result register.");
            throw std::runtime_error("BoolNode does not have a result register.");
        }
        interm_code_model::Register bool_reg = bool_node->result_register.value();
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            bool_node->interm_code_list.value().begin(),
            bool_node->interm_code_list.value().end());
        spdlog::debug("Added {} intermediate codes from BoolNode to StatNode.", bool_node->interm_code_list.value().size());
        // 2. get 3 new labels: one for the start of the loop, one for the end of the loop, and one for the true branch
        std::string loop_start_label = simulator.get_new_temp_label(scope_id);
        std::string loop_end_label = simulator.get_new_temp_label(scope_id);
        std::string loop_true_label = simulator.get_new_temp_label(scope_id);
        // 3. generate a goto_if code, destination is the true label, criteria is the boolean expression result register, AND label it with the loop start label
        auto goto_if_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO_IF,
            interm_code_model::Operand(bool_reg),
            interm_code_model::Operand(loop_true_label, interm_code_model::OperandType::LABEL),
            std::nullopt,
            loop_start_label);
        this->interm_code_list->push_back(goto_if_code);
        spdlog::debug("Generated GOTO_IF code for STAT_WHILE with label: {}", loop_true_label);
        // 4. generate a goto code to the end label(this is to skip the false branch)
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            interm_code_model::Operand(loop_end_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to end label: {}", loop_end_label);
        // 5. add an empty intermediate code for the start of the loop, label it with the loop start label
        auto loop_start_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            loop_start_label);
        this->interm_code_list->push_back(loop_start_code);
        spdlog::debug("Added empty intermediate code for start of loop with label: {}", loop_start_label);
        // 6. add the intermediate code of the stat list node to the list
        if (stat_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                stat_node->interm_code_list.value().begin(),
                stat_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from StatListNode to StatNode.", stat_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("StatListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("StatListNode does not have an initialized intermediate code list.");
        }
        // 7. add an empty intermediate code for the end of the loop, label it with the loop end label
        auto loop_end_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            loop_end_label);
        this->interm_code_list->push_back(loop_end_code);
        spdlog::debug("Added empty intermediate code for end of STAT_WHILE with label: {}", loop_end_label);
    }
    else if (node_type == ast_model::ASTNodeType::STAT_RETURN)
    {
        // cast 1 to ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[1]);
        if (!expr_node)
        {
            spdlog::error("STAT_RETURN must have an ExprNode as subnode.");
            throw std::runtime_error("Invalid STAT_RETURN structure.");
        }
        auto expr_reg = expr_node->result_register;
        if (!expr_reg.has_value())
        {
            spdlog::error("ExprNode does not have a result register.");
            throw std::runtime_error("ExprNode does not have a result register.");
        }
        // take in the intermediate code of the expression node
        if (expr_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                expr_node->interm_code_list.value().begin(),
                expr_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from ExprNode to StatNode.", expr_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("ExprNode does not have an initialized intermediate code list.");
            throw std::runtime_error("ExprNode does not have an initialized intermediate code list.");
        }
        // store the return value in R1 register
        interm_code_model::Register return_reg(interm_code_model::RegisterType::TYPE_R_GENERAL, 1);
        auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            interm_code_model::Operand(return_reg),
            interm_code_model::Operand(expr_reg.value()));
        this->interm_code_list->push_back(interm_code);
        spdlog::debug("Generated intermediate code for return statement: return {}", expr_node->value.value_or("N/A"));
        // add a goto code, destination: RA register
        interm_code_model::Register ra_reg(interm_code_model::RegisterType::TYPE_RA, 0);
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            interm_code_model::Operand(ra_reg));
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to return address register: {}", ra_reg.toString());
    }
    else if (node_type == ast_model::ASTNodeType::STAT_COMPOUND)
    {
        // note in this language, solely compound statements will not change scope, just add the intermediate code of the subnodes
        // cast 1 to StatListNode
        auto stat_list_node = std::dynamic_pointer_cast<StatListNode>(subnodes[1]);
        if (!stat_list_node)
        {
            spdlog::error("STAT_COMPOUND must have a StatListNode as subnode.");
            throw std::runtime_error("Invalid STAT_COMPOUND structure.");
        }
        // add the intermediate code of the stat list node to the list
        if (stat_list_node->interm_code_list.has_value())
        {
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                stat_list_node->interm_code_list.value().begin(),
                stat_list_node->interm_code_list.value().end());
            spdlog::debug("Added {} intermediate codes from StatListNode to StatNode.", stat_list_node->interm_code_list.value().size());
        }
        else
        {
            spdlog::error("StatListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("StatListNode does not have an initialized intermediate code list.");
        }
    }
    else if (node_type == ast_model::ASTNodeType::STAT_FUNC_CALL)
    {
        // the most complicated one, cast 0 to IDNode, 2 to RegArgListNode
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        auto arg_list_node = std::dynamic_pointer_cast<RealArgListNode>(subnodes[2]);
        // 1. iterate through the arguments and generate intermediate code for each argument
        int arg_index = 1;
        for (const auto &arg_node : arg_list_node->subnodes)
        {
            auto casted_arg_node = std::dynamic_pointer_cast<RealArgNode>(arg_node);
            // get the result register of the argument node
            if (!casted_arg_node || !casted_arg_node->result_register.has_value())
            {
                spdlog::error("RealArgNode does not have a result register.");
                throw std::runtime_error("RealArgNode does not have a result register.");
            }
            // generate the intermediate code for passing T to R reg
            interm_code_model::Register arg_reg = casted_arg_node->result_register.value();
            interm_code_model::Register R_reg(interm_code_model::RegisterType::TYPE_R_GENERAL, arg_index);
            auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::ASSIGN,
                interm_code_model::Operand(R_reg),
                interm_code_model::Operand(arg_reg));
            this->interm_code_list->push_back(interm_code);
            spdlog::debug("Generated intermediate code for function argument {}: {} to register {}", arg_index, casted_arg_node->value.value_or("N/A"), R_reg.toString());
            // increment the argument index
            arg_index++;
        }
        // 2. save the current state
        auto save_state_code = simulator.save_scope_state(scope_id);
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            save_state_code.begin(),
            save_state_code.end());
        spdlog::debug("Saved current scope state for function call.");
        // 3. get a new temp label for return address, assign to RA register
        std::string return_label = simulator.get_new_temp_label(scope_id);
        interm_code_model::Register ra_reg(interm_code_model::RegisterType::TYPE_RA, 0);
        auto assign_ra_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            interm_code_model::Operand(ra_reg),
            interm_code_model::Operand(return_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(assign_ra_code);
        spdlog::debug("Assigned return address label {} to RA register {}", return_label, ra_reg.toString());
        // 4. generate a goto code to the function label
        // get the function label from simulator
        std::string func_label = simulator.get_func_label(id_node->value);
        interm_code_model::Operand func_label_operand(func_label, interm_code_model::OperandType::LABEL);
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            func_label_operand);
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to function label: {}", id_node->value);
        // 5. add an empty intermediate code for the return address, label it with the return label
        auto return_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            return_label);
        this->interm_code_list->push_back(return_code);
        spdlog::debug("Added empty intermediate code for return address with label: {}", return_label);
        // 6. restore the state after the function call
        auto restore_state_code = simulator.restore_scope_state(scope_id);
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            restore_state_code.begin(),
            restore_state_code.end());
        spdlog::debug("Restored scope state after function call.");
    }
    else
    {
        throw std::runtime_error("STAT_NODE type not implemented for intermediate code generation: " + ast_node_type_to_string(node_type));
    }
}

void ast_model::ExprNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for ExprNode.");
    // init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // Implement intermediate code generation for ExprNode
    if (node_type == ASTNodeType::EXPR_CONST)
    {
        // 1. this is a constant expression, so we just need to create a new register and assign the constant value to it
        if (!this->value.has_value())
        {
            spdlog::error("Constant expression must have a value.");
            throw std::runtime_error("Constant expression must have a value.");
        }
        interm_code_model::Register const_reg = simulator.get_new_treg(scope_id);
        interm_code_model::Operand const_operand(this->value.value(), interm_code_model::OperandType::CONSTANT);
        auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            const_reg,
            const_operand);
        this->interm_code_list->push_back(interm_code);
        // set the result register to the constant register
        this->result_register = const_reg;
        spdlog::debug("Generated intermediate code for constant expression: {} with register {}", this->value.value(), const_reg.toString());
    }
    else if (node_type == ASTNodeType::EXPR_VAR)
    {
        // 2. get the variable register and set as the result register, no need to generate intermediate code
        if (!this->value.has_value())
        {
            spdlog::error("Variable expression must have a value.");
            throw std::runtime_error("Variable expression must have a value.");
        }
        interm_code_model::Register var_reg = simulator.get_var_reg(this->value.value(), scope_id);
        // set the result register to the variable register
        this->result_register = var_reg;
        spdlog::debug("Generated intermediate code for variable expression: {} with register {}", this->value.value(), var_reg.toString());
    }
    else if (node_type == ASTNodeType::EXPR_ATOMIC_TEMP)
    {
        // 3. this is an atomic temporary expression for enforcing mul priority(E_MUL -> E_ATOMIC_TEMP), so we just need to create a new register and assign the value to it
        // cast 0 to ExprNode
        auto sub_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        // inherit the subnode's intermediate code list & result register
        this->interm_code_list = sub_expr_node->interm_code_list;
        this->result_register = sub_expr_node->result_register;
        if (!this->result_register.has_value())
        {
            spdlog::error("Atomic temporary expression must have a result register.");
            throw std::runtime_error("Atomic temporary expression must have a result register.");
        }
        spdlog::debug("Generated intermediate code for atomic temporary expression with register {}", this->result_register.value().toString());
    }
    else if (node_type == ASTNodeType::EXPR_MUL_TEMP){
        // 4. another temporary expression for enforcing mul priority(E -> E_MUL), so we just need to create a new register and assign the value to it
        // cast 0 to ExprNode
        auto sub_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        // inherit the subnode's intermediate code list & result register
        this->interm_code_list = sub_expr_node->interm_code_list;
        this->result_register = sub_expr_node->result_register;
        if (!this->result_register.has_value())
        {
            spdlog::error("Mul temporary expression must have a result register.");
            throw std::runtime_error("Mul temporary expression must have a result register.");
        }
        spdlog::debug("Generated intermediate code for mul temporary expression with register {}", this->result_register.value().toString());
    }
    else if (node_type == ASTNodeType::EXPR_PAREN_NOCONST) {
        // 5. E_ATOMIC -> (E), just inherit from subnode 1
        // cast 1 to ExprNode
        auto sub_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[1]);
        if (!sub_expr_node)
        {
            spdlog::error("Parenthesis expression must have a subnode.");
            throw std::runtime_error("Invalid parenthesis expression structure.");
        }
        // inherit the subnode's intermediate code list & result register
        this->interm_code_list = sub_expr_node->interm_code_list;
        this->result_register = sub_expr_node->result_register;
        if (!this->result_register.has_value())
        {
            spdlog::error("Parenthesis expression must have a result register.");
            throw std::runtime_error("Parenthesis expression must have a result register.");
        }
        spdlog::debug("Generated intermediate code for parenthesis expression with register {}", this->result_register.value().toString());
    }
    else if (node_type == ASTNodeType::EXPR_ARITH_NOCONST){
        // 6. E -> E + E | E * E, cast 0 & 2 to ExprNode, 1 to TerminalNode
        auto left_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        auto right_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        auto op_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[1]);
        if (!left_expr_node || !right_expr_node || !op_node)
        {
            spdlog::error("Arithmetic expression must have two ExprNodes and a TerminalNode as subnodes.");
            throw std::runtime_error("Invalid arithmetic expression structure.");
        }
        auto left_reg = left_expr_node->result_register.value();
        auto right_reg = right_expr_node->result_register.value();
        // inherit the intermediate code list from the left & right subnodes
        this->interm_code_list = left_expr_node->interm_code_list;
        this->interm_code_list.value().insert(
            this->interm_code_list.value().end(),
            right_expr_node->interm_code_list.value().begin(),
            right_expr_node->interm_code_list.value().end());
        // reuse the left register for the result
        this->result_register = left_reg;
        // generate interm code for the operation
        if (op_node->node_type == ASTNodeType::MUL)
        {
            // generate multiplication code
            auto code_mul = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::MUL,
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(right_reg));
            this->interm_code_list->push_back(code_mul);
        }
        else if (op_node->node_type == ASTNodeType::ADD)
        {
            // generate addition code
            auto code_add = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::ADD,
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(right_reg));
            this->interm_code_list->push_back(code_add);
        }
        else
        {
            spdlog::error("Invalid arithmetic operation: {}", op_node->value);
            throw std::runtime_error("Invalid arithmetic operation.");
        }
        spdlog::debug("Generated intermediate code for arithmetic expression: {} {} {} with register {}", left_expr_node->value.value_or("N/A"), op_node->value, right_expr_node->value.value_or("N/A"), left_reg.toString());
    }
    else if (node_type == ASTNodeType::EXPR_ARRAY) {
        // 7. E -> ID[E], cast 0 to TerminalNode, 2 to ExprNode
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        auto index_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        if (!id_node || !index_expr_node)
        {
            spdlog::error("Array expression must have a TerminalNode and an ExprNode as subnodes.");
            throw std::runtime_error("Invalid array expression structure.");
        }
        throw std::runtime_error("Array expression is not implemented for intermediate code generation yet.");
    }
    else if (node_type == ASTNodeType::EXPR_FUNC)
    {
        // 8. E -> ID(R), cast 0 to TerminalNode, 2 to RealArgListNode
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        auto arg_list_node = std::dynamic_pointer_cast<RealArgListNode>(subnodes[2]);
        if (!id_node || !arg_list_node)
        {
            spdlog::error("Function call expression must have a TerminalNode and a RealArgListNode as subnodes.");
            throw std::runtime_error("Invalid function call expression structure.");
        }
        // like the STAT_FUNC_CALL, we need to generate intermediate code for the function call
        // 1. iterate through the arguments and generate intermediate code for each argument
        int arg_index = 1;
        for (const auto &arg_node : arg_list_node->subnodes)
        {
            auto casted_arg_node = std::dynamic_pointer_cast<RealArgNode>(arg_node);
            // get the result register of the argument node
            if (!casted_arg_node || !casted_arg_node->result_register.has_value())
            {
                spdlog::error("RealArgNode does not have a result register.");
                throw std::runtime_error("RealArgNode does not have a result register.");
            }
            // generate the intermediate code for passing T to R reg
            interm_code_model::Register arg_reg = casted_arg_node->result_register.value();
            interm_code_model::Register R_reg(interm_code_model::RegisterType::TYPE_R_GENERAL, arg_index);
            auto interm_code = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::ASSIGN,
                interm_code_model::Operand(R_reg),
                interm_code_model::Operand(arg_reg));
            this->interm_code_list->push_back(interm_code);
            spdlog::debug("Generated intermediate code for function argument {}: {} to register {}", arg_index, casted_arg_node->value.value_or("N/A"), R_reg.toString());
            // increment the argument index
            arg_index++;
        }
        // 2. save the current state
        auto save_state_code = simulator.save_scope_state(scope_id);
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            save_state_code.begin(),
            save_state_code.end());
        spdlog::debug("Saved current scope state for function call.");
        // 3. get a new temp label for return address, assign to RA register
        std::string return_label = simulator.get_new_temp_label(scope_id);
        interm_code_model::Register ra_reg(interm_code_model::RegisterType::TYPE_RA, 0);
        auto assign_ra_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::ASSIGN,
            interm_code_model::Operand(ra_reg),
            interm_code_model::Operand(return_label, interm_code_model::OperandType::LABEL));
        this->interm_code_list->push_back(assign_ra_code);
        spdlog::debug("Assigned return address label {} to RA register {}", return_label, ra_reg.toString());
        // 4. generate a goto code to the function label
        // get the function label from simulator
        std::string func_label = simulator.get_func_label(id_node->value);
        interm_code_model::Operand func_label_operand(func_label, interm_code_model::OperandType::LABEL);
        auto goto_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::GOTO,
            func_label_operand);
        this->interm_code_list->push_back(goto_code);
        spdlog::debug("Generated GOTO code to function label: {}", id_node->value);
        // 5. add an empty intermediate code for the return address, label it with the return label
        auto return_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            return_label);
        this->interm_code_list->push_back(return_code);
        spdlog::debug("Added empty intermediate code for return address with label: {}", return_label);
        // 6. restore the state after the function call
        auto restore_state_code = simulator.restore_scope_state(scope_id);
        this->interm_code_list->insert(
            this->interm_code_list->end(),
            restore_state_code.begin(),
            restore_state_code.end());
        spdlog::debug("Restored scope state after function call.");
        // 7. set the result register to R1 register
        interm_code_model::Register r1_reg(interm_code_model::RegisterType::TYPE_R_GENERAL, 1);
        this->result_register = r1_reg;
        spdlog::debug("Set result register to R1 register: {}", r1_reg.toString());
    }
    else
    {
        throw std::runtime_error("EXPR_NODE type not implemented for intermediate code generation: " + ast_node_type_to_string(node_type));
    }
}

void ast_model::TerminalNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    // pass
    spdlog::info("TerminalNode does not generate intermediate code, it is only used for semantic analysis.");
};

void ast_model::BoolNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for BoolNode.");
    // init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // Implement intermediate code generation for BoolNode
    if (node_type == ASTNodeType::BOOL_EXPR)
    {
        // B -> E
        // cast 0 to ExprNode and inherit its intermediate code list & result register
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        if (!expr_node)
        {
            spdlog::error("BoolNode must have an ExprNode as subnode.");
            throw std::runtime_error("Invalid BoolNode structure.");
        }
        this->interm_code_list = expr_node->interm_code_list;
        this->result_register = expr_node->result_register;
        if (!this->result_register.has_value())
        {
            spdlog::error("BoolNode must have a result register.");
            throw std::runtime_error("BoolNode must have a result register.");
        }
        spdlog::debug("Generated intermediate code for BoolNode from ExprNode with register {}", this->result_register.value().toString());
    }
    else if (node_type == ASTNodeType::BOOL_OP)
    {
        // B -> E OP E
        // cast 0 & 2 to ExprNode, 1 to TerminalNode
        auto left_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        auto right_expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[2]);
        auto op_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[1]);
        if (!left_expr_node || !right_expr_node || !op_node)
        {
            spdlog::error("BoolNode must have two ExprNodes and a TerminalNode as subnodes.");
            throw std::runtime_error("Invalid BoolNode structure.");
        }
        auto left_reg = left_expr_node->result_register.value();
        auto right_reg = right_expr_node->result_register.value();
        // inherit the intermediate code list from the left & right subnodes
        this->interm_code_list = left_expr_node->interm_code_list;
        this->interm_code_list.value().insert(
            this->interm_code_list.value().end(),
            right_expr_node->interm_code_list.value().begin(),
            right_expr_node->interm_code_list.value().end());
        // reuse the left register for the result
        this->result_register = left_reg;
        // generate interm code for the operation
        // vaild op: <, <=, ==
        if (op_node->value == "<")
        {
            // generate less than code
            auto code_lt = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::IS_SMALLER,
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(right_reg));
            this->interm_code_list->push_back(code_lt);
        }
        else if (op_node->value == "==")
        {
            // generate equal to code
            auto code_eq = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::IS_EQUAL,
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(right_reg));
            this->interm_code_list->push_back(code_eq);
        }
        else if (op_node->value == "<=")
        {
            // generate less than or equal to code
            auto code_le = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::IS_LESS_EQUAL,
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(left_reg),
                interm_code_model::Operand(right_reg));
            this->interm_code_list->push_back(code_le);
        }
        else
        {
            spdlog::error("Invalid boolean operation: {}", op_node->value);
            throw std::runtime_error("Invalid boolean operation.");
        }
    }
};

void ast_model::RealArgListNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    // alike DeclListNode, gather the intermediate code from subnodes
    spdlog::info("Generating intermediate code for RealArgListNode.");
    // init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // 1. check subnode count
    int subnode_count = subnodes.size();
    if (subnode_count == 0)
    {
        spdlog::debug("RealArgListNode has no subnodes, this is an empty argument list.");
        return; // empty argument list
    }
    else
    {
        // cast 0 to RealArgListNode and 1 to RealArgNode
        auto real_arg_list_node = std::dynamic_pointer_cast<RealArgListNode>(subnodes[0]);
        auto real_arg_node = std::dynamic_pointer_cast<RealArgNode>(subnodes[1]);
        if (!real_arg_list_node || !real_arg_node)
        {
            spdlog::error("RealArgListNode must have a RealArgListNode and a RealArgNode as subnodes.");
            throw std::runtime_error("Invalid RealArgListNode structure.");
        }
        // 2. add the intermediate code of the subnode 0 to the current node's intermediate code list
        if (real_arg_list_node->interm_code_list.has_value())
        {
            this->interm_code_list = real_arg_list_node->interm_code_list.value();
        }
        else
        {
            spdlog::error("RealArgListNode does not have an initialized intermediate code list.");
            throw std::runtime_error("RealArgListNode does not have an initialized intermediate code list.");
        }
        // 3. append the intermediate code of the real argument node
        if (real_arg_node->interm_code_list.has_value())
        {
            auto real_arg_interm_code_list = real_arg_node->interm_code_list.value();
            this->interm_code_list->insert(
                this->interm_code_list->end(),
                real_arg_interm_code_list.begin(),
                real_arg_interm_code_list.end());
            spdlog::debug("Added {} intermediate codes from RealArgNode to RealArgListNode.", real_arg_interm_code_list.size());
        }
        else
        {
            spdlog::error("RealArgNode does not have an initialized intermediate code list.");
            throw std::runtime_error("RealArgNode does not have an initialized intermediate code list.");
        }
        spdlog::info("Intermediate code generation for RealArgListNode completed with {} subnodes.", subnode_count);
    }
};

void ast_model::RealArgNode::generate_intermediate_code(
    LogicalEnvSimulator &simulator)
{
    spdlog::info("Generating intermediate code for RealArgNode.");
    // init the intermediate code list
    this->interm_code_list = std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>();
    // Implement intermediate code generation for RealArgNode
    if (node_type == ASTNodeType::RARG_EXPR)
    {
        // inherit interm code & result register from ExprNode
        auto expr_node = std::dynamic_pointer_cast<ExprNode>(subnodes[0]);
        if (!expr_node)
        {
            spdlog::error("RealArgNode must have an ExprNode as subnode.");
            throw std::runtime_error("Invalid RealArgNode structure.");
        }
        this->interm_code_list = expr_node->interm_code_list;
        this->result_register = expr_node->result_register;
    }
    else if (node_type == ASTNodeType::RARG_ARRAY) {
        // find the result register from simulator
        auto id_node = std::dynamic_pointer_cast<TerminalNode>(subnodes[0]);
        std::string arr_name = id_node->value;
        auto arr_reg = simulator.get_arr_reg(arr_name, scope_id);
        this->result_register = arr_reg;
    }
    else
    {
        throw std::runtime_error("RealArgNode type not implemented for intermediate code generation: " + ast_node_type_to_string(node_type));
    }
};
