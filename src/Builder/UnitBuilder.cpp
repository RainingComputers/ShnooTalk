#include "UnitBuilder.hpp"

using namespace icode;

UnitBuilder::UnitBuilder(OperandBuilder& opBuilder)
  : opBuilder(opBuilder)
{
}

Unit UnitBuilder::unitPairFromIntLiteral(int value, DataType dtype)
{
    Operand op = opBuilder.createIntLiteralOperand(dtype, value);
    icode::TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitPairFromFloatLiteral(float value, DataType dtype)
{
    Operand op = opBuilder.createFloatLiteralOperand(dtype, value);
    icode::TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitPairFromTypeDescription(TypeDescription& typeDescription,
                                                                const Token& nameToken)
{
    Operand op = opBuilder.operandFromTypeDescription(typeDescription, nameToken);
    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitFromEnum(int enumValue)
{
    icode::Operand op = opBuilder.createIntLiteralOperand(icode::INT, enumValue);
    
    icode::TypeDescription typeDescription = icode::typeDescriptionFromDataType(icode::INT);
    typeDescription.setProperty(IS_ENUM);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitFromDefineDescription(const DefineDescription& defineDescription)
{
    icode::Operand op;

    if (defineDescription.dtype == icode::INT)
        op = opBuilder.createIntLiteralOperand(icode::INT, defineDescription.val.integer);
    else
        op = opBuilder.createFloatLiteralOperand(icode::FLOAT, defineDescription.val.floating);

    icode::TypeDescription typeDescription = icode::typeDescriptionFromDataType(defineDescription.dtype);
    typeDescription.setProperty(IS_DEFINE);

    return Unit(op, typeDescription);
}