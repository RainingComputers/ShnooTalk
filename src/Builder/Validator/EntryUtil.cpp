#include "EntryUtil.hpp"

bool allOperandEqualDataType(const icode::Entry& e)
{
    return (e.op1.dtype == e.op2.dtype) && (e.op2.dtype == e.op3.dtype || none(e.op3));
}

bool allOperandNonVoidBaseDataType(const icode::Entry& e)
{
    return nonVoidBaseDataType(e.op1) && (nonVoidBaseDataType(e.op2) || none(e.op2)) &&
           (nonVoidBaseDataType(e.op3) || none(e.op3));
}

bool allOperandNonPointer(const icode::Entry& e)
{
    return e.op1.isNotPointer() && e.op2.isNotPointer() && (e.op3.isNotPointer() || none(e.op3));
}

bool noOperand(const icode::Entry& e)
{
    return none(e.op1) && none(e.op2) && none(e.op3);
}

bool oneOperand(const icode::Entry& e)
{
    return notNone(e.op1) && none(e.op2) && none(e.op3);
}

bool twoOperand(const icode::Entry& e)
{
    return notNone(e.op1) && notNone(e.op2) && none(e.op3);
}

bool threeOperand(const icode::Entry& e)
{
    return notNone(e.op1) && notNone(e.op2) && notNone(e.op3);
}