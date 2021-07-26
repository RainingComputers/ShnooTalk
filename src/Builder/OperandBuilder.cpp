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

Operand OperandBuilder::createStringDataOperand(const std::string& name, unsigned int size)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = name;
    temp.operandType = STR_DATA;
    temp.val.size = size;
    temp.dtype = icode::UI8;

    return temp;
}

Operand OperandBuilder::createBytesOperand(unsigned int address)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.bytes = address;
    temp.operandType = BYTES;
    temp.dtype = AUTO_INT;

    return temp;
}

Operand OperandBuilder::createVarOperand(DataType dtype, const std::string& symbol, bool global, bool ptr)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = symbol;
    temp.dtype = dtype;

    if (global)
        temp.operandType = GBL_VAR;
    else if (ptr)
        temp.operandType = PTR;
    else
        temp.operandType = VAR;

    return temp;
}

Operand OperandBuilder::createRetPointerOperand(DataType dtype)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.operandType = RET_VALUE;

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

Operand OperandBuilder::createFloatLiteralOperand(DataType dtype, float literal)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.floating = literal;
    temp.dtype = dtype;
    temp.operandType = LITERAL;

    return temp;
}

Operand OperandBuilder::createIntLiteralOperand(DataType dtype, int literal)
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
    return createVarOperand(typeDescription.dtype,
                            name,
                            typeDescription.checkProperty(icode::IS_GLOBAL),
                            typeDescription.checkProperty(icode::IS_PTR));
}