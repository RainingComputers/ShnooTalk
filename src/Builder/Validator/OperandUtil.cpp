#include "OperandUtil.hpp"

bool validDataType(const icode::Operand& op)
{
    return op.dtype <= icode::VOID;
}

bool nonVoidBaseDataType(const icode::Operand& op)
{
    return op.dtype < icode::STRUCT;
}

bool voidDataType(const icode::Operand& op)
{
    return op.dtype == icode::VOID;
}

bool temp(const icode::Operand& op)
{
    return op.operandType == icode::TEMP;
}

bool none(const icode::Operand& op)
{
    return op.operandType == icode::NONE;
}

bool notNone(const icode::Operand& op)
{
    return op.operandType != icode::NONE;
}

bool mod(const icode::Operand& op)
{
    return op.operandType == icode::MODULE;
}

bool var(const icode::Operand& op)
{
    return op.operandType == icode::VAR;
}

bool label(const icode::Operand& op)
{
    return op.operandType == icode::LABEL;
}

bool retValue(const icode::Operand& op)
{
    return op.operandType == icode::RET_VALUE;
}

bool notPointer(const icode::Operand& op)
{
    return op.operandType == icode::TEMP || op.operandType == icode::LITERAL || op.operandType == icode::VAR ||
           op.operandType == icode::GBL_VAR || op.operandType == icode::CALLEE_RET_VAL;
}
