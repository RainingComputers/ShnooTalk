#include "NameMangle.hpp"
#include "TypeCheck.hpp"

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

ModuleDescription* DescriptionFinder::getModuleFromType(const TypeDescription& type)
{
    return &modulesMap.at(type.moduleName);
}

ModuleDescription* DescriptionFinder::getModuleFromUnit(const Unit& unit)
{
    return &modulesMap.at(unit.moduleName());
}

ModuleDescription* DescriptionFinder::getModuleFromToken(const Token& aliasToken)
{
    const std::string& alias = aliasToken.toString();

    std::string moduleName;

    if (!workingModule->getModuleNameFromAlias(alias, moduleName))
        console.compileErrorOnToken("Use does not exist", aliasToken);

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
    long defineValue;

    if (!workingModule->getIntDefine(nameToken.toString(), defineValue))
        if (!rootModule.getIntDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromIntLiteral(defineValue);
    return true;
}

bool DescriptionFinder::getFloatDefine(const Token& nameToken, Unit& returnValue)
{
    double defineValue;

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

unsigned int DescriptionFinder::getDataTypeSizeFromToken(const Token& nameToken)
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

    if (workingModule->getExternFunction(nameToken.toString(), functionDescription))
        return functionDescription;

    if (workingModule->getFunction(nameMangle(nameToken, workingModule->name), functionDescription))
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

bool isSameParamsType(const FunctionDescription& function, const std::vector<Unit>& params)
{
    for (size_t i = 0; i < params.size(); i += 1)
    {
        const TypeDescription& actualParamType = params[i].type();
        const TypeDescription& formalParamType = function.symbols.at(function.parameters[i]);

        if (!isSameTypeDescription(formalParamType, actualParamType))
            return false;
    }

    return true;
}

std::pair<std::string, FunctionDescription> DescriptionFinder::getFunctionByParamTypes(const Token& token,
                                                                                       const TypeDescription& type,
                                                                                       const std::vector<Unit>& params)
{
    for (auto pair : workingModule->functions)
    {
        const FunctionDescription& function = pair.second;
        const std::string& functionName = pair.first;

        if (!isSameTypeDescription(function.functionReturnType, type))
            continue;

        if (function.numParameters() != params.size())
            continue;

        if (!isSameParamsType(function, params))
            continue;

        return std::pair<std::string, FunctionDescription>(functionName, function);
    }

    console.compileErrorOnToken("Cannot find function with matching params", token);
}

bool DescriptionFinder::isAllNamesStructFields(const std::vector<Token>& nameTokens, const Unit& structUnit)
{
    const StructDescription& structDescription = workingModule->structures.at(structUnit.dtypeName());

    for (auto nameToken : nameTokens)
        if (!structDescription.fieldExists(nameToken.toString()))
            return false;

    return true;
}
