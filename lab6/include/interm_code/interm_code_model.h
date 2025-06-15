#ifndef INTERM_CODE_MODEL_H
#define INTERM_CODE_MODEL_H
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <optional>

namespace interm_code_model
{

    // forward declaration
    enum class RegisterType;
    enum class OperationType;
    enum class LogicalMemSpaceType;
    enum class OperandType;
    class LogicalAddress;
    class Register;
    class Operand;
    class IntermediateCode;

    enum class RegisterType
    {
        TYPE_T_GENERAL,
        TYPE_R_GENERAL,
        TYPE_RA // special register for function return address
    };

    RegisterType stringToRegisterType(const std::string &type);
    std::string registerTypeToString(RegisterType type);

    enum class OperationType
    {
        ASSIGN,
        LOAD,
        STORE,
        GOTO,
        GOTO_IF,
        ADD,
        MUL,
        IS_SMALLER,
        IS_EQUAL,
        EMPTY,
    };

    OperationType stringToOperationType(const std::string &type);
    std::string operationTypeToString(OperationType type);

    enum class LogicalMemSpaceType
    {
        CODE,
        DATA,
        STACK
    };

    LogicalMemSpaceType stringToLogicalMemSpaceType(const std::string &type);
    std::string logicalMemSpaceTypeToString(LogicalMemSpaceType type);

    enum class OperandType
    {
        REGISTER,
        LOGICAL_ADDRESS,
        CONSTANT,
        LABEL // For labels in GOTO operations
    };

    struct LogicalAddress
    {
        LogicalMemSpaceType memSpaceType;
        int offset;

        LogicalAddress() = default;
        LogicalAddress(LogicalMemSpaceType memSpaceType, int offset) : memSpaceType(memSpaceType), offset(offset) {}
        std::string toString() const;

        bool operator==(const LogicalAddress &other) const;
    };

    struct Register
    {
        RegisterType type;
        int id;

        Register() = default;
        Register(RegisterType type, int id) : type(type), id(id) {}
        std::string toString() const;

        bool operator==(const Register &other) const;
    };
    struct Operand
    {
        OperandType type;
        std::optional<Register> reg;
        std::optional<LogicalAddress> logicalAddress;
        std::optional<std::string> constant;
        std::optional<std::string> label; // For labels in GOTO operations

        Operand(Register reg) : type(OperandType::REGISTER), reg(reg) {}
        Operand(LogicalAddress logicalAddress) : type(OperandType::LOGICAL_ADDRESS), logicalAddress(logicalAddress) {}
        Operand(std::string string_value, OperandType type)
        {
            if (type == OperandType::CONSTANT)
            {
                this->type = OperandType::CONSTANT;
                constant = string_value;
            }
            else if (type == OperandType::LABEL)
            {
                this->type = OperandType::LABEL;
                label = string_value;
            }
            else
            {
                throw std::invalid_argument("Invalid OperandType for string value");
            }
        }
        std::string toString() const;

        bool operator==(const Operand &other) const;
    };

    struct IntermediateCode
    {
        OperationType type;
        std::optional<Operand> operand_1;
        std::optional<Operand> operand_2;
        std::optional<Operand> operand_3;
        std::optional<std::string> label; // For labels in GOTO operations

        IntermediateCode(OperationType type, std::optional<Operand> operand_1 = std::nullopt,
                         std::optional<Operand> operand_2 = std::nullopt,
                         std::optional<Operand> operand_3 = std::nullopt, std::optional<std::string> label = std::nullopt)
            : type(type), operand_1(operand_1), operand_2(operand_2), operand_3(operand_3), label(label) {}
        std::string toString() const;

        bool operator==(const IntermediateCode &other) const;
    };
};

namespace std
{
    // hash functions for the above structs
    template <>
    struct hash<interm_code_model::LogicalAddress>;

    template <>
    struct hash<interm_code_model::Register>;

    template <>
    struct hash<interm_code_model::Operand>;

    template <>
    struct hash<interm_code_model::IntermediateCode>;
};

#endif // !INTERM_CODE_MODEL_H