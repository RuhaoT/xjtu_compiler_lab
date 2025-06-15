#include "interm_code/interm_code_model.h"
#include <stdexcept>
#include <sstream> // Required for ostringstream
#include <functional> // Required for std::hash
#include <iomanip> // Required for std::setw, std::left

namespace interm_code_model
{

    RegisterType stringToRegisterType(const std::string &type)
    {
        if (type == "T")
            return RegisterType::TYPE_T_GENERAL;
        if (type == "R")
            return RegisterType::TYPE_R_GENERAL;
        if (type == "RA")
            return RegisterType::TYPE_RA; // special register for function return address
        throw std::invalid_argument("Unknown RegisterType string: " + type);
    }

    std::string registerTypeToString(RegisterType type)
    {
        switch (type)
        {
        case RegisterType::TYPE_T_GENERAL:
            return "T";
        case RegisterType::TYPE_R_GENERAL:
            return "R";
        case RegisterType::TYPE_RA:
            return "RA"; // special register for function return address
        default:
            throw std::invalid_argument("Unknown RegisterType enum value");
        }
    }

    OperationType stringToOperationType(const std::string &type)
    {
        if (type == "ASSIGN")
            return OperationType::ASSIGN;
        if (type == "LOAD")
            return OperationType::LOAD;
        if (type == "STORE")
            return OperationType::STORE;
        if (type == "GOTO")
            return OperationType::GOTO;
        if (type == "GOTO_IF")
            return OperationType::GOTO_IF;
        if (type == "ADD")
            return OperationType::ADD;
        if (type == "MUL")
            return OperationType::MUL;
        if (type == "IS_SMALLER")
            return OperationType::IS_SMALLER;
        if (type == "IS_EQUAL")
            return OperationType::IS_EQUAL;
        if (type == "IS_LESS_EQUAL")
            return OperationType::IS_LESS_EQUAL; // For less than or equal to operation
        if (type == "EMPTY")
            return OperationType::EMPTY; // For empty operations, if needed
        throw std::invalid_argument("Unknown OperationType string: " + type);
    }

    std::string operationTypeToString(OperationType type)
    {
        switch (type)
        {
        case OperationType::ASSIGN:
            return "ASSIGN";
        case OperationType::LOAD:
            return "LOAD";
        case OperationType::STORE:
            return "STORE";
        case OperationType::GOTO:
            return "GOTO";
        case OperationType::GOTO_IF:
            return "GOTO_IF";
        case OperationType::ADD:
            return "ADD";
        case OperationType::MUL:
            return "MUL";
        case OperationType::IS_SMALLER:
            return "IS_SMALLER";
        case OperationType::IS_EQUAL:
            return "IS_EQUAL";
        case OperationType::IS_LESS_EQUAL:
            return "IS_LESS_EQUAL"; // For less than or equal to operation
        case OperationType::EMPTY:
            return "EMPTY"; // For empty operations, if needed
        default:
            throw std::invalid_argument("Unknown OperationType enum value");
        }
    }

    LogicalMemSpaceType stringToLogicalMemSpaceType(const std::string &type)
    {
        if (type == "CODE")
            return LogicalMemSpaceType::CODE;
        if (type == "DATA")
            return LogicalMemSpaceType::DATA;
        if (type == "STACK")
            return LogicalMemSpaceType::STACK;
        throw std::invalid_argument("Unknown LogicalMemSpaceType string: " + type);
    }

    std::string logicalMemSpaceTypeToString(LogicalMemSpaceType type)
    {
        switch (type)
        {
        case LogicalMemSpaceType::CODE:
            return "CODE";
        case LogicalMemSpaceType::DATA:
            return "DATA";
        case LogicalMemSpaceType::STACK:
            return "STACK";
        default:
            throw std::invalid_argument("Unknown LogicalMemSpaceType enum value");
        }
    }

    std::string LogicalAddress::toString() const
    {
        return logicalMemSpaceTypeToString(memSpaceType) + "[" + std::to_string(offset) + "]";
    }

    bool LogicalAddress::operator==(const LogicalAddress &other) const
    {
        return memSpaceType == other.memSpaceType && offset == other.offset;
    }

    std::string Register::toString() const
    {
        return registerTypeToString(type) + std::to_string(id);
    }

    bool Register::operator==(const Register &other) const
    {
        return type == other.type && id == other.id;
    }

    std::string Operand::toString() const
    {
        switch (type)
        {
        case OperandType::REGISTER:
            return reg.value().toString();
        case OperandType::LOGICAL_ADDRESS:
            return logicalAddress.value().toString();
        case OperandType::CONSTANT:
            return constant.value();
        case OperandType::LABEL: // Added case for LABEL
            return label.value();
        default:
            throw std::invalid_argument("Unknown OperandType enum value");
        }
    }

    bool Operand::operator==(const Operand &other) const
    {
        if (type != other.type)
            return false;
        switch (type)
        {
        case OperandType::REGISTER:
            return reg == other.reg;
        case OperandType::LOGICAL_ADDRESS:
            return logicalAddress == other.logicalAddress;
        case OperandType::CONSTANT:
            return constant == other.constant;
        case OperandType::LABEL: // Added case for LABEL
            return label == other.label;
        default:
            return false; // Should not happen
        }
    }

    std::string IntermediateCode::toString() const
    {
        std::ostringstream oss;
        oss << std::left; // Align to the left

        // Field 1: Operation Type
        oss << std::setw(8) << operationTypeToString(type);

        // Field 2: Operand 1
        std::string op1_str = "-";
        if (operand_1) {
            op1_str = operand_1.value().toString();
        }
        oss << std::setw(10) << op1_str;

        // Field 3: Operand 2
        std::string op2_str = "-";
        if (operand_2) {
            op2_str = operand_2.value().toString();
        }
        oss << std::setw(10) << op2_str;

        // Field 4: Operand 3
        std::string op3_str = "-";
        if (operand_3) {
            op3_str = operand_3.value().toString();
        }
        oss << std::setw(10) << op3_str;

        // Field 5: Label
        oss << "label: " << std::setw(10) << (label.has_value() ? label.value() : "-");
        
        return oss.str();
    }

    bool IntermediateCode::operator==(const IntermediateCode &other) const
    {
        return type == other.type &&
               operand_1 == other.operand_1 &&
               operand_2 == other.operand_2 &&
               operand_3 == other.operand_3 &&
               label == other.label; // Compare label as well
    }

} // namespace interm_code_model

namespace std
{
    // hash functions for the above structs
    template <>
    struct hash<interm_code_model::LogicalAddress>
    {
        size_t operator()(const interm_code_model::LogicalAddress &obj) const
        {
            size_t h1 = std::hash<int>()(static_cast<int>(obj.memSpaceType));
            size_t h2 = std::hash<int>()(obj.offset);
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };

    template <>
    struct hash<interm_code_model::Register>
    {
        size_t operator()(const interm_code_model::Register &obj) const
        {
            size_t h1 = std::hash<int>()(static_cast<int>(obj.type));
            size_t h2 = std::hash<int>()(obj.id);
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };

    template <>
    struct hash<interm_code_model::Operand>
    {
        size_t operator()(const interm_code_model::Operand &obj) const
        {
            size_t h_type = std::hash<int>()(static_cast<int>(obj.type));
            size_t h_val = 0;
            switch (obj.type)
            {
            case interm_code_model::OperandType::REGISTER:
                if (obj.reg)
                    h_val = std::hash<interm_code_model::Register>()(obj.reg.value());
                break;
            case interm_code_model::OperandType::LOGICAL_ADDRESS:
                if (obj.logicalAddress)
                    h_val = std::hash<interm_code_model::LogicalAddress>()(obj.logicalAddress.value());
                break;
            case interm_code_model::OperandType::CONSTANT:
                if (obj.constant)
                    h_val = std::hash<std::string>()(obj.constant.value());
                break;
            case interm_code_model::OperandType::LABEL: // Added case for LABEL
                if (obj.label)
                    h_val = std::hash<std::string>()(obj.label.value());
                break;
            }
            return h_type ^ (h_val << 1);
        }
    };

    template <>
    struct hash<interm_code_model::IntermediateCode>
    {
        size_t operator()(const interm_code_model::IntermediateCode &obj) const
        {
            size_t h_type = std::hash<int>()(static_cast<int>(obj.type));
            size_t h_op1 = 0;
            size_t h_op2 = 0;
            size_t h_op3 = 0;
            size_t h_label = 0;

            if (obj.operand_1)
                h_op1 = std::hash<interm_code_model::Operand>()(obj.operand_1.value());
            if (obj.operand_2)
                h_op2 = std::hash<interm_code_model::Operand>()(obj.operand_2.value());
            if (obj.operand_3)
                h_op3 = std::hash<interm_code_model::Operand>()(obj.operand_3.value());
            if (obj.label)
                h_label = std::hash<std::string>()(obj.label.value());

            size_t seed = h_type;
            seed ^= h_op1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h_op2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h_op3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h_label + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
} // namespace std