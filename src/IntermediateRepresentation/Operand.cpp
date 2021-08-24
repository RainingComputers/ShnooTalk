#include "Operand.hpp"

namespace icode
{
    Operand::Operand()
    {
        operandId = 0;
        dtype = VOID;
        operandType = NONE;
    }
    bool Operand::isPointer() const
    {
        return operandType == PTR || operandType == TEMP_PTR;
    }

    bool Operand::isNotPointer() const
    {
        return operandType == icode::TEMP || operandType == icode::LITERAL || operandType == icode::VAR ||
               operandType == icode::GBL_VAR || operandType == icode::CALLEE_RET_VAL;
    }

    bool Operand::isUserPointer() const
    {
        return operandType == PTR;
    }

    bool Operand::isValidForPointerAssignment() const
    {
        return operandType == VAR || operandType == GBL_VAR || operandType == TEMP_PTR || operandType == PTR;
    }
}