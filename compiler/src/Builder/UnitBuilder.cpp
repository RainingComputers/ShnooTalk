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

Unit UnitBuilder::unitFromIntLiteral(long value)
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

Unit UnitBuilder::unitFromFloatLiteral(double value)
{
    Operand op = opBuilder.createFloatLiteralOperand(AUTO_FLOAT, value);
    TypeDescription typeDescription = typeDescriptionFromDataType(AUTO_FLOAT);

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromEnum(const EnumDescription& enumDescription)
{
    Operand op = opBuilder.createIntLiteralOperand(ENUM, enumDescription.value);
    TypeDescription typeDescription = typeDescriptionFromDataType(ENUM);

    typeDescription.dtypeName = enumDescription.dtypeName;
    typeDescription.moduleName = enumDescription.moduleName;

    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromTypeDescription(const TypeDescription& typeDescription, const std::string& name)
{
    Operand op = opBuilder.operandFromTypeDescription(typeDescription, name);
    return Unit(typeDescription, op);
}

Unit UnitBuilder::unitFromUnitList(const std::vector<Unit>& unitList)
{
    TypeDescription type = unitList[0].type();

    DimensionType dimType = FIXED_DIM;

    if (type.isStringLtrl())
        dimType = STRING_LTRL_DIM;

    type = prependDimension(type, unitList.size(), dimType);

    if (type.moduleName == "")
        type.moduleName = workingModule->name;

    return Unit(type, unitList).clearProperties();
}

Unit UnitBuilder::unitFromStringDataKey(const std::string& key)
{
    int charCount = workingModule->stringsDataCharCounts.at(key);

    std::vector<int> dimensions;
    dimensions.push_back(charCount);

    TypeDescription stringType = typeDescriptionFromDataType(icode::UI8);
    stringType = createArrayTypeDescription(stringType, dimensions, icode::STRING_LTRL_DIM);
    stringType.becomeStringLtrl();
    stringType.moduleName = rootModule.name;

    size_t size = charCount * getDataTypeSize(UI8);
    Operand opr = opBuilder.createStringDataOperand(key, size);

    return Unit(stringType, opr);
}
