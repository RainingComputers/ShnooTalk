#include "ValueBuilder.hpp"

using namespace icode;

ValueBuilder::ValueBuilder(OperandBuilder& opBuilder)
  : opBuilder(opBuilder)
{
}

OperandDescriptionPair ValueBuilder::operandDescPairFromIntLiteral(int value, DataType dtype)
{
    Operand op = opBuilder.createIntLiteralOperand(dtype, value);
    icode::TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return OperandDescriptionPair(op, typeDescription);
}

OperandDescriptionPair ValueBuilder::operandDescPairFromFloatLiteral(float value, DataType dtype)
{
    Operand op = opBuilder.createFloatLiteralOperand(dtype, value);
    icode::TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return OperandDescriptionPair(op, typeDescription);
}

OperandDescriptionPair ValueBuilder::operandDescPairFromTypeDesc(TypeDescription& typeDescription,
                                                                const token::Token& nameToken)
{
    Operand op = opBuilder.operandFromTypeDescription(typeDescription, nameToken);
    return OperandDescriptionPair(op, typeDescription);
}

OperandDescriptionPair ValueBuilder::operandDescPairFromEnum(int enumValue)
{
    icode::Operand op = opBuilder.createIntLiteralOperand(icode::INT, enumValue);
    
    icode::TypeDescription typeDescription = icode::typeDescriptionFromDataType(icode::INT);
    typeDescription.setProperty(IS_ENUM);

    return OperandDescriptionPair(op, typeDescription);
}

OperandDescriptionPair ValueBuilder::operandDescPairFromDefine(const DefineDescription& defineDescription)
{
    icode::Operand op;

    if (defineDescription.dtype == icode::INT)
        op = opBuilder.createIntLiteralOperand(icode::INT, defineDescription.val.integer);
    else
        op = opBuilder.createFloatLiteralOperand(icode::FLOAT, defineDescription.val.floating);

    icode::TypeDescription typeDescription = icode::typeDescriptionFromDataType(defineDescription.dtype);
    typeDescription.setProperty(IS_DEFINE);

    return OperandDescriptionPair(op, typeDescription);
}