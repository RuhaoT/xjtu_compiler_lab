#ifndef AST_MODEL_H
#define AST_MODEL_H

#include <string>
#include <vector>
#include <memory>
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
    EXPR_CONST,
    EXPR_VAR,
    EXPR_FUNC,
    EXPR_ARRAY,
    EXPR_ARITH_NOCONST,
    EXPR_PAREN_NOCONST,
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

ASTNodeType string_to_ast_node_type(const std::string& node_type) {
    if (node_type == "PROGRAM") return ASTNodeType::PROGRAM;
    else if (node_type == "DECL_LIST") return ASTNodeType::DECL_LIST;
    else if (node_type == "DECL_VAR") return ASTNodeType::DECL_VAR;
    else if (node_type == "DECL_FUNC") return ASTNodeType::DECL_FUNC;
    else if (node_type == "DECL_ARRAY") return ASTNodeType::DECL_ARRAY;
    else if (node_type == "TYPE") return ASTNodeType::TYPE;
    else if (node_type == "ARG_LIST") return ASTNodeType::ARG_LIST;
    else if (node_type == "ARG_VAR") return ASTNodeType::ARG_VAR;
    else if (node_type == "ARG_FUNC") return ASTNodeType::ARG_FUNC;
    else if (node_type == "ARG_ARRAY") return ASTNodeType::ARG_ARRAY;
    else if (node_type == "STAT_LIST") return ASTNodeType::STAT_LIST;
    else if (node_type == "STAT_ASSIGN") return ASTNodeType::STAT_ASSIGN;
    else if (node_type == "STAT_ARRAY_ASSIGN") return ASTNodeType::STAT_ARRAY_ASSIGN;
    else if (node_type == "STAT_IF") return ASTNodeType::STAT_IF;
    else if (node_type == "STAT_IF_ELSE") return ASTNodeType::STAT_IF_ELSE;
    else if (node_type == "STAT_WHILE") return ASTNodeType::STAT_WHILE;
    else if (node_type == "STAT_RETURN") return ASTNodeType::STAT_RETURN;
    else if (node_type == "STAT_COMPOUND") return ASTNodeType::STAT_COMPOUND;
    else if (node_type == "STAT_FUNC_CALL") return ASTNodeType::STAT_FUNC_CALL;
    else if (node_type == "EXPR_CONST") return ASTNodeType::EXPR_CONST;
    else if (node_type == "EXPR_VAR") return ASTNodeType::EXPR_VAR;
    else if (node_type == "EXPR_FUNC") return ASTNodeType::EXPR_FUNC;
    else if (node_type == "EXPR_ARRAY") return ASTNodeType::EXPR_ARRAY;
    else if (node_type == "EXPR_ARITH_NOCONST") return ASTNodeType::EXPR_ARITH_NOCONST;
    else if (node_type == "EXPR_PAREN_NOCONST")
        return ASTNodeType::EXPR_PAREN_NOCONST;
    else if (node_type == "BOOL_OP") return ASTNodeType::BOOL_OP;
    else if (node_type == "BOOL_EXPR") return ASTNodeType::BOOL_EXPR;
    else if (node_type == "RARG_EXPR") return ASTNodeType::RARG_EXPR;
    else if (node_type == "RARG_FUNC") return ASTNodeType::RARG_FUNC;
    else if (node_type == "RARG_ARRAY") return ASTNodeType::RARG_ARRAY;
    else if (node_type == "RARG_LIST") return ASTNodeType::RARG_LIST;
    else if (node_type == "ID") return ASTNodeType::ID;
    else if (node_type == "NUM") return ASTNodeType::NUM;
    else if (node_type == "FLO") return ASTNodeType::FLO;
    else if (node_type == "ADD") return ASTNodeType::ADD;
    else if (node_type == "MUL") return ASTNodeType::MUL;
    else if (node_type == "ROP") return ASTNodeType::ROP;
    else if (node_type == "ASG") return ASTNodeType::ASG;
    else if (node_type == "LPA") return ASTNodeType::LPA;
    else if (node_type == "RPA") return ASTNodeType::RPA;
    else if (node_type == "LBK") return ASTNodeType::LBK;
    else if (node_type == "RBK") return ASTNodeType::RBK;
    else if (node_type == "LBR") return ASTNodeType::LBR;
    else if (node_type == "RBR") return ASTNodeType::RBR;
    else if (node_type == "CMA") return ASTNodeType::CMA;
    else if (node_type == "SCO") return ASTNodeType::SCO;
    else if (node_type == "INT") return ASTNodeType::INT;
    else if (node_type == "FLOAT") return ASTNodeType::FLOAT;
    else if (node_type == "IF") return ASTNodeType::IF;
    else if (node_type == "ELSE") return ASTNodeType::ELSE;
    else if (node_type == "WHILE") return ASTNodeType::WHILE;
    else if (node_type == "RETURN") return ASTNodeType::RETURN;
    else if (node_type == "INPUT") return ASTNodeType::INPUT;
    else if (node_type == "VOID") return ASTNodeType::VOID;
    else {
        throw std::invalid_argument("Unknown ASTNodeType string: " + node_type);
    }
}

std::string ast_node_type_to_string(ASTNodeType node_type) {
    switch (node_type) {
        case ASTNodeType::PROGRAM: return "PROGRAM";
        case ASTNodeType::DECL_LIST: return "DECL_LIST";
        case ASTNodeType::DECL_VAR: return "DECL_VAR";
        case ASTNodeType::DECL_FUNC: return "DECL_FUNC";
        case ASTNodeType::DECL_ARRAY: return "DECL_ARRAY";
        case ASTNodeType::TYPE: return "TYPE";
        case ASTNodeType::ARG_LIST: return "ARG_LIST";
        case ASTNodeType::ARG_VAR: return "ARG_VAR";
        case ASTNodeType::ARG_FUNC: return "ARG_FUNC";
        case ASTNodeType::ARG_ARRAY: return "ARG_ARRAY";
        case ASTNodeType::STAT_LIST: return "STAT_LIST";
        case ASTNodeType::STAT_ASSIGN: return "STAT_ASSIGN";
        case ASTNodeType::STAT_ARRAY_ASSIGN: return "STAT_ARRAY_ASSIGN";
        case ASTNodeType::STAT_IF: return "STAT_IF";
        case ASTNodeType::STAT_IF_ELSE: return "STAT_IF_ELSE";
        case ASTNodeType::STAT_WHILE: return "STAT_WHILE";
        case ASTNodeType::STAT_RETURN: return "STAT_RETURN";
        case ASTNodeType::STAT_COMPOUND: return "STAT_COMPOUND";
        case ASTNodeType::STAT_FUNC_CALL: return "STAT_FUNC_CALL";
        case ASTNodeType::EXPR_CONST: return "EXPR_CONST";
        case ASTNodeType::EXPR_VAR: return "EXPR_VAR";
        case ASTNodeType::EXPR_FUNC: return "EXPR_FUNC";
        case ASTNodeType::EXPR_ARRAY: return "EXPR_ARRAY";
        case ASTNodeType::EXPR_ARITH_NOCONST: return "EXPR_ARITH_NOCONST";
        case ASTNodeType::EXPR_PAREN_NOCONST: return "EXPR_PAREN_NOCONST";
        case ASTNodeType::BOOL_OP: return "BOOL_OP";
        case ASTNodeType::BOOL_EXPR: return "BOOL_EXPR";
        case ASTNodeType::RARG_EXPR: return "RARG_EXPR";
        case ASTNodeType::RARG_FUNC: return "RARG_FUNC";
        case ASTNodeType::RARG_ARRAY: return "RARG_ARRAY";
        case ASTNodeType::RARG_LIST: return "RARG_LIST";
        case ASTNodeType::ID: return "ID";
        case ASTNodeType::NUM: return "NUM";
        case ASTNodeType::FLO: return "FLO";
        case ASTNodeType::ADD: return "ADD";
        case ASTNodeType::MUL: return "MUL";
        case ASTNodeType::ROP: return "ROP";
        case ASTNodeType::ASG: return "ASG";
        case ASTNodeType::LPA: return "LPA";
        case ASTNodeType::RPA: return "RPA";
        case ASTNodeType::LBK: return "LBK";
        case ASTNodeType::RBK: return "RBK";
        case ASTNodeType::LBR: return "LBR";
        case ASTNodeType::RBR: return "RBR";
        case ASTNodeType::CMA: return "CMA";
        case ASTNodeType::SCO: return "SCO";
        case ASTNodeType::INT: return "INT";
        case ASTNodeType::FLOAT: return "FLOAT";
        case ASTNodeType::IF: return "IF";
        case ASTNodeType::ELSE: return "ELSE";
        case ASTNodeType::WHILE: return "WHILE";
        case ASTNodeType::RETURN: return "RETURN";
        case ASTNodeType::INPUT: return "INPUT";
        case ASTNodeType::VOID: return "VOID";
    }
    throw std::invalid_argument("Unknown ASTNodeType enum value");
};

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
    int scope_id = -1; // 作用域id

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
    int scope_id = -1; // 作用域id

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

    void semantic_action(int scope_id, std::shared_ptr<SymbolTable> symbol_table, std::shared_ptr<ScopeTable> scope_table) override;

    std::string to_string() const override;
};

}
#endif // !AST_MODEL_H