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

Operand OperandBuilder::createTempOperand(DataType dtype, const std::string& dtype_name)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.dtypeName = dtype_name;
    temp.operandType = TEMP;

    return temp;
}

Operand OperandBuilder::createPointerOperand(DataType dtype, const std::string& dtype_name)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.dtypeName = dtype_name;
    temp.operandType = TEMP_PTR;

    return temp;
}

Operand OperandBuilder::createStringDataOperand(const std::string& name, unsigned int size)
{
    Operand temp;
    temp.operandId = getId();
    temp.name = name;
    temp.dtype = icode::UI8;
    temp.operandType = STR_DATA;
    temp.val.size = size;

    return temp;
}

Operand OperandBuilder::createLiteralAddressOperand(unsigned int address)
{
    Operand temp;
    temp.operandId = getId();
    temp.val.address = address;
    temp.operandType = ADDR;

    return temp;
}

Operand OperandBuilder::createVarOperand(DataType dtype,
                                         const std::string& dtype_name,
                                         const std::string& symbol,
                                         bool global,
                                         bool ptr)
{
    Operand temp;
    temp.operandId = getId();
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

Operand OperandBuilder::createRetPointerOperand(DataType dtype, const std::string& dtype_name)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.dtypeName = dtype_name;
    temp.operandType = RET_PTR;

    return temp;
}

Operand OperandBuilder::createCalleeRetValOperand(DataType dtype, const std::string& dtype_name)
{
    Operand temp;
    temp.operandId = getId();
    temp.dtype = dtype;
    temp.dtypeName = dtype_name;
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

Operand OperandBuilder::operandFromTypeDescription(const TypeDescription& typeDescription,
                                                   const token::Token& nameToken)
{
    return createVarOperand(typeDescription.dtype,
                            typeDescription.dtypeName,
                            nameToken.toString(),
                            typeDescription.checkProperty(icode::IS_GLOBAL),
                            typeDescription.checkProperty(icode::IS_PTR));
}