#include "TypeDescriptionUtil.hpp"

#include "UnitBuilder.hpp"

using namespace icode;

UnitBuilder::UnitBuilder(ModuleDescription& rootModule, OperandBuilder& opBuilder)
    : rootModule(rootModule)
    , opBuilder(opBuilder)
{
}

void UnitBuilder::setWorkingModule(ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

Unit UnitBuilder::unitFromIntLiteral(int value)
{
    Operand op = opBuilder.createIntLiteralOperand(AUTO_INT, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(AUTO_INT);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromCharLiteral(char value)
{
    Operand op = opBuilder.createIntLiteralOperand(UI8, value);
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

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromUnitList(const std::vector<Unit>& unitList)
{
    TypeDescription type = unitList[0].type();

    DimensionType dimType = FIXED_DIM;

    if (type.isStringLtrl())
        dimType = STRING_LTRL_DIM;

    type = prependDimension(type, unitList.size(), dimType);

    return Unit(type, unitList);
}

int UnitBuilder::getCharCountFromStringDataKey(const std::string& key)
{
    auto resultItem = rootModule.stringsDataCharCounts.find(key);

    if (resultItem != rootModule.stringsDataCharCounts.end())
        return resultItem->second;

    return workingModule->stringsDataCharCounts.at(key);
}

Unit UnitBuilder::unitFromStringDataKey(const std::string& key)
{
    int charCount = getCharCountFromStringDataKey(key);

    std::vector<int> dimensions;
    dimensions.push_back(charCount);

    TypeDescription stringType = typeDescriptionFromDataType(icode::UI8);
    stringType = createArrayTypeDescription(stringType, dimensions, icode::STRING_LTRL_DIM);
    stringType.becomeString();
    stringType.moduleName = rootModule.name;

    size_t size = charCount * getDataTypeSize(UI8);
    Operand opr = opBuilder.createStringDataOperand(key, size);

    return Unit(stringType, opr);
}
