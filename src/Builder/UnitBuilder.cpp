#include "TypeDescriptionUtil.hpp"

#include "UnitBuilder.hpp"

using namespace icode;

UnitBuilder::UnitBuilder(OperandBuilder& opBuilder)
  : opBuilder(opBuilder)
{
}

Unit UnitBuilder::unitFromIntLiteral(int value, DataType dtype)
{
    Operand op = opBuilder.createIntLiteralOperand(dtype, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitFromFloatLiteral(float value, DataType dtype)
{
    Operand op = opBuilder.createFloatLiteralOperand(dtype, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(dtype);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitPairFromTypeDescription(TypeDescription& typeDescription, const std::string& name)
{
    Operand op = opBuilder.operandFromTypeDescription(typeDescription, name);
    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitFromEnum(int enumValue)
{
    Operand op = opBuilder.createIntLiteralOperand(INT, enumValue);

    TypeDescription typeDescription = typeDescriptionFromDataType(INT);
    typeDescription.setProperty(IS_ENUM);

    return Unit(op, typeDescription);
}

Unit UnitBuilder::unitFromDefineDescription(const DefineDescription& defineDescription)
{
    Operand op;

    if (defineDescription.dtype == INT)
        op = opBuilder.createIntLiteralOperand(INT, defineDescription.val.integer);
    else
        op = opBuilder.createFloatLiteralOperand(FLOAT, defineDescription.val.floating);

    TypeDescription typeDescription = typeDescriptionFromDataType(defineDescription.dtype);
    typeDescription.setProperty(IS_DEFINE);

    return Unit(op, typeDescription);
}