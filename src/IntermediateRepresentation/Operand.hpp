#ifndef OPERAND_HPP
#define OPERAND_HPP

#include "DataType.hpp"
#include "TypeDescription.hpp"

namespace icode
{

    enum OperandType
    {
        TEMP,
        TEMP_PTR,
        ADDR,
        STR_DATA,
        VAR,
        GBL_VAR,
        PTR,
        RET_PTR,
        CALLEE_RET_VAL,
        LITERAL,
        LABEL,
        MODULE,
        NONE,
    };

    struct Operand
    {
        unsigned int operandId;

        union
        {
            int integer;
            float floating;
            unsigned int address;
            unsigned int size;
        } val;

        std::string name;
        DataType dtype;
        std::string dtypeName;
        OperandType operandType;

        bool operator<(const Operand& other) const;
        bool operator==(const Operand& other) const;
        void updateDataType(const TypeDescription& var);
        bool isPointer() const;
    };

    Operand createTempOperand(DataType dtype, const std::string& dtype_name, unsigned int id);

    Operand createPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id);

    Operand createStringDataOperand(const std::string& name, unsigned int size, unsigned int id);

    Operand createVarOperand(DataType dtype,
                             const std::string& dtype_name,
                             const std::string& symbol,
                             unsigned int id,
                             bool global = false,
                             bool ptr = false);

    Operand createRetPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id);

    Operand createCalleeRetValOperand(DataType dtype, const std::string& dtype_name, unsigned int id);

    Operand createLiteralOperand(DataType dtype, float literal, unsigned int id);

    Operand createLiteralOperand(DataType dtype, int literal, unsigned int id);

    Operand createLiteralAddressOperand(unsigned int address, unsigned int id);

    Operand createLabelOperand(const std::string& label, unsigned int id);

    Operand createModuleOperand(const std::string& module, unsigned int id);
}

#endif