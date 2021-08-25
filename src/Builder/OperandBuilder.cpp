#include "OperandBuilder.hpp"

using namespace icode;

OperandBuilder::OperandBuilder()
{
    idCounter = 0;
}

unsigned int OperandBuilder::getId()
{
    return idCounter++;
}

Operand OperandBuilder::createTempOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = TEMP;

    return temp;
}

Operand OperandBuilder::createPointerOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = TEMP_PTR;

    return temp;
}

Operand OperandBuilder::createStringDataOperand(const std::string& name, unsigned long size)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = name;
    temp.operandType = STR_DATA;
    temp.val.size = size;
    temp.dtype = icode::UI8;

    return temp;
}

Operand OperandBuilder::createBytesOperand(unsigned long address)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.bytes = address;
    temp.operandType = BYTES;
    temp.dtype = AUTO_INT;

    return temp;
}

Operand OperandBuilder::createVarOperand(DataType dtype, const std::string& name, OperandType type)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = name;
    temp.dtype = dtype;
    temp.operandType = type;

    return temp;
}

Operand OperandBuilder::createRetValueOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = RET_VALUE;

    return temp;
}

Operand OperandBuilder::createRetPointerOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = RET_PTR;

    return temp;
}

Operand OperandBuilder::createCalleeRetValOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = CALLEE_RET_VAL;

    return temp;
}

Operand OperandBuilder::createCalleeRetPointerOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = CALLEE_RET_PTR;

    return temp;
}

Operand OperandBuilder::createFloatLiteralOperand(DataType dtype, double literal)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.floating = literal;
    temp.dtype = dtype;
    temp.operandType = LITERAL;

    return temp;
}

Operand OperandBuilder::createIntLiteralOperand(DataType dtype, long literal)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.integer = literal;
    temp.dtype = dtype;
    temp.operandType = LITERAL;

    return temp;
}

Operand OperandBuilder::createLabelOperand(const std::string& label)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = label;
    temp.operandType = LABEL;

    return temp;
}

Operand OperandBuilder::createModuleOperand(const std::string& module)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = module;
    temp.operandType = MODULE;

    return temp;
}

Operand OperandBuilder::operandFromTypeDescription(const TypeDescription& typeDescription, const std::string& name)
{
    if (typeDescription.checkProperty(IS_GLOBAL))
        return createVarOperand(typeDescription.dtype, name, GBL_VAR);

    else if (typeDescription.checkProperty(IS_PTR))
        return createVarOperand(typeDescription.dtype, name, PTR);

    else
        return createVarOperand(typeDescription.dtype, name, VAR);
}