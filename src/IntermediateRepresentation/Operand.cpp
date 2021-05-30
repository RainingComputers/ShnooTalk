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

    void Operand::updateDataType(const TypeDescription& var)
    {
        dtype = var.dtype;
        dtypeName = var.dtypeName;
    }

    bool Operand::isPointer() const
    {
        return operandType == icode::PTR || operandType == icode::TEMP_PTR || operandType == icode::RET_PTR;
    }
}