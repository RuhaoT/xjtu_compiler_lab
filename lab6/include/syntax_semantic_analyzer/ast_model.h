#ifndef AST_MODEL_H
#define AST_MODEL_H

#include <string>
#include <vector>
#include <memory>
#include "interm_code/interm_code_model.h"
#include "symbol_table.h"
#include "scope_table.h"

namespace ast_model {

const int INT_MEMORY_SIZE = 4; // int类型占用的内存大小
const int FLOAT_MEMORY_SIZE = 8; // float类型占用的内存大小
const int MEMORY_ADDRESS_SIZE = 8; // 内存地址占用的字节数

struct ASTNodeContent;
struct ProgramNode;
struct DeclListNode;
struct DeclNode;
struct TypeNode;
struct ArgListNode;
struct ArgNode;
struct StatListNode;
struct StatNode;
struct ExprNode;
struct BoolNode;
struct RealArgNode;
struct RealArgListNode;
struct TerminalNode;

enum class ASTNodeType {
    PROGRAM,
    DECL_LIST,
    DECL_VAR,
    DECL_FUNC,
    DECL_ARRAY,
    TYPE,
    ARG_LIST,
    ARG_VAR,
    ARG_FUNC,
    ARG_ARRAY,
    STAT_LIST,
    STAT_ASSIGN,
    STAT_ARRAY_ASSIGN,
    STAT_IF,
    STAT_IF_ELSE,
    STAT_WHILE,
    STAT_RETURN,
    STAT_COMPOUND,
    STAT_FUNC_CALL,
    STAT_IFELSE_MS, // 用于处理if-else的多分支情况，表示完全匹配的IF-ELSE语句
    STAT_IFELSE_UMS, // 用于处理if-else的多分支情况，表示不完全匹配的IF-ELSE语句
    STAT_IF_ELSE_CHAIN,
    EXPR_CONST,
    EXPR_VAR,
    EXPR_FUNC,
    EXPR_ARRAY,
    EXPR_ARITH_NOCONST,
    EXPR_PAREN_NOCONST,
    EXPR_MUL_TEMP, // used for solving the ambiguity of multiplication and addition in expressions
    EXPR_ATOMIC_TEMP, // used for solving the ambiguity of multiplication and addition in expressions
    BOOL_OP,
    BOOL_EXPR,
    RARG_EXPR,
    RARG_FUNC,
    RARG_ARRAY,
    RARG_LIST,
    ID,
    NUM,
    FLO,
    ADD,
    MUL,
    ROP,
    ASG,
    LPA,
    RPA,
    LBK,
    RBK,
    LBR,
    RBR,
    CMA,
    SCO,
    INT,
    FLOAT,
    IF,
    ELSE,
    WHILE,
    RETURN,
    INPUT,
    VOID,
};

ASTNodeType string_to_ast_node_type(const std::string& node_type);

std::string ast_node_type_to_string(ASTNodeType node_type);

std::shared_ptr<void> cast_ast_node_base_ptr_to_specific_node(
    const std::shared_ptr<ASTNodeContent>& node,
    const std::string& node_type
);

struct ASTNodeContent {
    ASTNodeType node_type; // 节点类型，用于标识节点的类型
    std::vector<std::shared_ptr<ASTNodeContent>> subnodes; // 子节点列表

    // 接收子节点
    virtual void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) = 0;

    // 执行语义动作
    virtual void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) = 0;

    // 将节点转换为字符串表示形式
    virtual std::string to_string() const = 0;

    virtual ~ASTNodeContent() = default; // 虚析构函数，确保派生类正确析构
};

// 一大堆派生类
struct ProgramNode : public ASTNodeContent {
    ProgramNode() {
        node_type = ASTNodeType::PROGRAM;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;

};

struct DeclListNode : public ASTNodeContent {

    int decl_count = 0; // 用于记录声明的数量
    std::vector<std::shared_ptr<DeclNode>> declarations;
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    DeclListNode() {
        node_type = ASTNodeType::DECL_LIST;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;

};

struct DeclNode : public ASTNodeContent {
    std::string data_type = ""; // 数据类型
    std::string var_name = ""; // 变量名
    std::optional<int> array_length; // 数组长度，可选
    std::optional<std::shared_ptr<ArgListNode>> arg_list; // 函数形参列表，可选
    std::optional<int> function_sub_scope; // 函数子作用域id，可选
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    DeclNode(const std::string& node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct TypeNode : public ASTNodeContent {
    std::string data_type = ""; // 数据类型

    TypeNode(){
        node_type = ASTNodeType::TYPE;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct ArgListNode : public ASTNodeContent {
    int arg_count = 0; // 用于记录形参的数量
    std::vector<std::shared_ptr<ArgNode>> arg_info; // 形参信息列表

    ArgListNode() {
        node_type = ASTNodeType::ARG_LIST;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct ArgNode : public ASTNodeContent {
    std::string data_type = ""; // 数据类型
    std::string var_name = ""; // 变量名

    ArgNode(const std::string& node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct StatListNode : public ASTNodeContent {
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    StatListNode() {
        node_type = ASTNodeType::STAT_LIST;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;

};

struct StatNode : public ASTNodeContent {
    int scope_id = -1; // 作用域id
    interm_code_model::LogicalAddress stat_address;
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    StatNode(const std::string& node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct ExprNode : public ASTNodeContent {
    std::string data_type = ""; // 数据类型
    std::optional<std::string> value = "";
    std::optional<interm_code_model::Register> result_register;
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    ExprNode(const std::string& node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct BoolNode : public ASTNodeContent {
    int scope_id = -1; // 作用域id
    std::optional<interm_code_model::Register> result_register;
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    BoolNode(const std::string&node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct RealArgNode : public ASTNodeContent {
    std::string data_type = ""; // 实参数据类型
    std::optional<std::string> value = ""; // 实参值
    std::optional<interm_code_model::Register> result_register;
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    RealArgNode(const std::string& node_type) {
        this->node_type = string_to_ast_node_type(node_type);
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

struct RealArgListNode : public ASTNodeContent {
    int arg_count = 0; // 用于记录实参的数量
    std::vector<std::shared_ptr<RealArgNode>> arg_info; // 实参列表
    int scope_id = -1; // 作用域id
    std::optional<std::vector<std::shared_ptr<interm_code_model::IntermediateCode>>> interm_code_list; // 中间代码列表，可选，比如声明函数时需要生成中间代码，但变量和数组声明不需要

    RealArgListNode() {
        node_type = ASTNodeType::RARG_LIST;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        this->subnodes = std::move(subnodes);
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;

};

struct TerminalNode : public ASTNodeContent {
    std::string value; // 终结符的值

    TerminalNode(const std::string& node_type, const std::string& value) {
        this->node_type = string_to_ast_node_type(node_type);
        this->value = value;
    }

    void subnode_takein(std::vector<std::shared_ptr<ASTNodeContent>> subnodes) override {
        // 终结符没有子节点
        this->subnodes.clear();
    }

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override {
        // 终结符不需要执行语义动作
        spdlog::warn("Semantic action called on TerminalNode, which should not have subnodes.");
    }

    std::string to_string() const override;
};

}
#endif // !AST_MODEL_H