#include "Operand.hpp"

namespace icode
{
    Operand::Operand()
    {
        operandId = 0;
        dtype = VOID;
        operandType = NONE;
    }

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
            case RET_VALUE:
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

    bool Operand::isPointer() const
    {
        return operandType == PTR || operandType == TEMP_PTR;
    }

    bool Operand::canPassAsMutable() const
    {
        return !(operandType == TEMP || operandType == STR_DATA || operandType == LITERAL);
    }

    bool Operand::isInvalidForInput() const
    {
        return !(operandType == VAR || operandType == GBL_VAR || operandType == TEMP_PTR || operandType == PTR);
    }
}