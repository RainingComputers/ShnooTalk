#include "NameMangle.hpp"

#include "DescriptionFinder.hpp"

using namespace icode;

DescriptionFinder::DescriptionFinder(ModuleDescription& rootModule,
                                     StringModulesMap& modulesMap,
                                     UnitBuilder& unitBuilder,
                                     Console& console)
  : rootModule(rootModule)
  , modulesMap(modulesMap)
  , unitBuilder(unitBuilder)
  , console(console)
{
}

void DescriptionFinder::setWorkingModule(ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

void DescriptionFinder::setWorkingFunction(FunctionDescription* functionDescription)
{
    workingFunction = functionDescription;
}

ModuleDescription* DescriptionFinder::getModuleFromUnit(const Unit& unit)
{
    return &modulesMap.at(unit.moduleName());
}

ModuleDescription* DescriptionFinder::getModuleFromToken(const Token& moduleNameToken)
{
    const std::string& moduleName = moduleNameToken.toString();

    if (!workingModule->useExists(moduleName))
        console.compileErrorOnToken("Module does not exist", moduleNameToken);

    return &modulesMap.at(moduleName);
}

bool DescriptionFinder::getLocal(const Token& nameToken, Unit& returnValue)
{
    TypeDescription typeDescription;

    if (!workingFunction->getSymbol(nameToken.toString(), typeDescription))
        return false;

    returnValue = unitBuilder.unitFromTypeDescription(typeDescription, nameToken.toString());
    return true;
}

bool DescriptionFinder::getGlobal(const Token& nameToken, Unit& returnValue)
{
    std::string mangledGlobalName = nameMangle(nameToken, rootModule.name);

    TypeDescription typeDescription;

    if (!rootModule.getGlobal(mangledGlobalName, typeDescription))
        return false;

    returnValue = unitBuilder.unitFromTypeDescription(typeDescription, mangledGlobalName);
    return true;
}

bool DescriptionFinder::getEnum(const Token& nameToken, Unit& returnValue)
{
    int enumValue;

    if (!workingModule->getEnum(nameToken.toString(), enumValue))
        if (!rootModule.getEnum(nameToken.toString(), enumValue))
            return false;

    returnValue = unitBuilder.unitFromEnum(enumValue);
    return true;
}

bool DescriptionFinder::getIntDefine(const Token& nameToken, Unit& returnValue)
{
    int defineValue;

    if (!workingModule->getIntDefine(nameToken.toString(), defineValue))
        if (!rootModule.getIntDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromIntLiteral(defineValue);
    return true;
}

bool DescriptionFinder::getFloatDefine(const Token& nameToken, Unit& returnValue)
{
    float defineValue;

    if (!workingModule->getFloatDefine(nameToken.toString(), defineValue))
        if (!rootModule.getFloatDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromFloatLiteral(defineValue);
    return true;
}

bool DescriptionFinder::getStringDefine(const Token& nameToken, Unit& returnValue)
{
    std::string defineValue;

    if (!workingModule->getStringDefine(nameToken.toString(), defineValue))
        if (!rootModule.getStringDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromStringDataKey(defineValue);
    return true;
}

Unit DescriptionFinder::getUnitFromToken(const Token& nameToken)
{
    Unit unit;

    if (getLocal(nameToken, unit))
        return unit;

    if (getGlobal(nameToken, unit))
        return unit;

    if (getEnum(nameToken, unit))
        return unit;

    if (getIntDefine(nameToken, unit))
        return unit;

    if (getFloatDefine(nameToken, unit))
        return unit;

    if (getStringDefine(nameToken, unit))
        return unit;

    console.compileErrorOnToken("Symbol does not exist", nameToken);
}

int DescriptionFinder::getDataTypeSizeFromToken(const Token& nameToken)
{
    const std::string& dataTypeString = nameToken.toString();

    DataType dtype = stringToDataType(dataTypeString);
    if (dtype != STRUCT)
        return getDataTypeSize(dtype);

    StructDescription structDescription;
    if (workingModule->getStruct(dataTypeString, structDescription))
        return structDescription.size;

    Unit unit = getUnitFromToken(nameToken);
    return unit.size();
}

FunctionDescription DescriptionFinder::getFunction(const Token& nameToken)
{
    FunctionDescription functionDescription;

    if ((*workingModule).getFunction(nameMangle(nameToken, workingModule->name), functionDescription))
        return functionDescription;

    if (rootModule.getFunction(nameMangle(nameToken, rootModule.name), functionDescription))
        return functionDescription;

    console.compileErrorOnToken("Function does not exist", nameToken);
}

std::vector<Unit> DescriptionFinder::getFormalParameters(const FunctionDescription& function)
{
    std::vector<Unit> formalParameters;

    for (const std::string& parameter : function.parameters)
    {
        TypeDescription paramType = function.symbols.at(parameter);
        formalParameters.push_back(unitBuilder.unitFromTypeDescription(paramType, parameter));
    }

    return formalParameters;
}

Unit DescriptionFinder::getReturnType()
{
    return unitBuilder.unitFromTypeDescription(workingFunction->functionReturnType, "return");
}
