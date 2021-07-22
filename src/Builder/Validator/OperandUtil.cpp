#include "OperandUtil.hpp"

bool validDataType(const icode::Operand& op)
{
    return op.dtype <= icode::VOID;
}

bool nonVoidBaseDataType(const icode::Operand& op)
{
    return op.dtype < icode::STRUCT;
}

bool nonVoidDataType(const icode::Operand& op)
{
    return op.dtype != icode::VOID;
}

bool none(const icode::Operand& op)
{
    return op.operandType == icode::NONE;
}

bool notNone(const icode::Operand& op)
{
    return op.operandType != icode::NONE;
}
