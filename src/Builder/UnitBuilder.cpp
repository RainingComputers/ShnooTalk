#include "TypeDescriptionUtil.hpp"

#include "UnitBuilder.hpp"

using namespace icode;

UnitBuilder::UnitBuilder(OperandBuilder& opBuilder)
  : opBuilder(opBuilder)
{
}

Unit UnitBuilder::unitFromIntLiteral(int value)
{
    Operand op = opBuilder.createIntLiteralOperand(AUTO_INT, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(AUTO_INT);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromFloatLiteral(float value)
{
    Operand op = opBuilder.createFloatLiteralOperand(AUTO_FLOAT, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(AUTO_FLOAT);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromTypeDescription(TypeDescription& typeDescription, const std::string& name)
{
    Operand op = opBuilder.operandFromTypeDescription(typeDescription, name);
    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromEnum(int enumValue)
{
    Operand op = opBuilder.createIntLiteralOperand(AUTO_INT, enumValue);

    TypeDescription typeDescription = typeDescriptionFromDataType(AUTO_INT);
    typeDescription.setProperty(IS_ENUM);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromDefineDescription(const DefineDescription& defineDescription)
{
    Operand op;

    if (defineDescription.dtype == AUTO_INT)
        op = opBuilder.createIntLiteralOperand(AUTO_INT, defineDescription.val.integer);
    else
        op = opBuilder.createFloatLiteralOperand(AUTO_FLOAT, defineDescription.val.floating);

    TypeDescription typeDescription = typeDescriptionFromDataType(defineDescription.dtype);
    typeDescription.setProperty(IS_DEFINE);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromUnitList(const std::vector<Unit>& unitList)
{
    TypeDescription type = unitList[0].type();

    DimensionType dimType = FIXED_DIM;

    if (type.checkProperty(IS_STRING_LTRL))
        dimType = STRING_LTRL_DIM;

    type = prependDimension(type, unitList.size(), dimType);

    return Unit(type, unitList);
}