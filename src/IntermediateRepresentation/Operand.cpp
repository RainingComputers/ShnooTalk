#include "Operand.hpp"

namespace icode
{
    bool Operand::operator<(const Operand& other) const
    {
        if (*this == other)
            return false;

        return operandId < other.operandId;
    }

    bool Operand::operator==(const Operand& other) const
    {
        if (operandType != other.operandType)
            return false;

        switch (operandType)
        {
            case TEMP:
            case TEMP_PTR:
            case RET_PTR:
            case CALLEE_RET_VAL:
                return operandId == other.operandId;
            case VAR:
            case GBL_VAR:
            case PTR:
            case STR_DATA:
            case LABEL:
            case MODULE:
                return name == other.name;
            case ADDR:
            case LITERAL:
                return val.size == other.val.size;
            case NONE:
                return false;
        }

        return false;
    }

    void Operand::updateDtype(const VariableDescription& var)
    {
        dtype = var.dtype;
        dtypeName = var.dtypeName;
    }

    bool Operand::isPointer() const
    {
        return operandType == icode::PTR || operandType == icode::TEMP_PTR || operandType == icode::RET_PTR;
    }

    Operand createTempOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = TEMP;

        return temp;
    }

    Operand createPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = TEMP_PTR;

        return temp;
    }

    Operand createStringDataOperand(const std::string& name, unsigned int size, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = name;
        temp.dtype = icode::UI8;
        temp.operandType = STR_DATA;
        temp.val.size = size;

        return temp;
    }

    Operand createLiteralAddressOperand(unsigned int address, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.address = address;
        temp.operandType = ADDR;

        return temp;
    }

    Operand createVarOperand(DataType dtype,
                             const std::string& dtype_name,
                             const std::string& symbol,
                             unsigned int id,
                             bool global,
                             bool ptr)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = symbol;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;

        if (global)
            temp.operandType = GBL_VAR;
        else if (ptr)
            temp.operandType = PTR;
        else
            temp.operandType = VAR;

        return temp;
    }

    Operand createRetPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = RET_PTR;

        return temp;
    }

    Operand createCalleeRetValOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = CALLEE_RET_VAL;

        return temp;
    }

    Operand createLiteralOperand(DataType dtype, float literal, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.floating = literal;
        temp.dtype = dtype;
        temp.operandType = LITERAL;

        return temp;
    }

    Operand createLiteralOperand(DataType dtype, int literal, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.integer = literal;
        temp.dtype = dtype;
        temp.operandType = LITERAL;

        return temp;
    }

    Operand createLabelOperand(const std::string& label, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = label;
        temp.operandType = LABEL;

        return temp;
    }

    Operand createModuleOperand(const std::string& module, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = module;
        temp.operandType = MODULE;

        return temp;
    }
}