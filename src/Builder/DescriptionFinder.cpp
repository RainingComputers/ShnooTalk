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
    const std::string alias = aliasToken.toString();

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
    EnumDescription enumDescription;

    if (!workingModule->getEnum(nameToken.toString(), enumDescription))
        if (!rootModule.getEnum(nameToken.toString(), enumDescription))
            return false;

    returnValue = unitBuilder.unitFromEnum(enumDescription);
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

FunctionDescription DescriptionFinder::getFunction(const Token& nameToken)
{
    FunctionDescription functionDescription;

    std::string incompleteFunctionModule;
    if (workingModule->getIncompleteFunctionModule(nameToken.toString(), incompleteFunctionModule))
        return workingModule->externFunctions.at(nameMangle(nameToken, incompleteFunctionModule));

    if (workingModule->getExternFunction(nameToken.toString(), functionDescription))
        return functionDescription;

    if (workingModule->getFunction(nameMangle(nameToken, workingModule->name), functionDescription))
        return functionDescription;

    if (workingModule->getFunction(nameToken.toString(), functionDescription))
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
    if (function.numParameters() != params.size())
        return false;

    for (size_t i = 0; i < params.size(); i += 1)
    {
        const TypeDescription actualParamType = params[i].type();
        const TypeDescription formalParamType = function.symbols.at(function.parameters[i]);

        if (!isSameTypeDescription(formalParamType, actualParamType))
            return false;
    }

    return true;
}

std::pair<std::string, FunctionDescription> DescriptionFinder::getFunctionByParamTypes(const Token& token,
                                                                                       const TypeDescription& type,
                                                                                       const std::vector<Unit>& params)
{
    for (auto functionName : workingModule->definedFunctions)
    {
        const FunctionDescription function = workingModule->functions.at(functionName);

        if (!isSameTypeDescription(function.functionReturnType, type))
            continue;

        if (!isSameParamsType(function, params))
            continue;

        return std::pair<std::string, FunctionDescription>(functionName, function);
    }

    console.compileErrorOnToken("Cannot find function with matching params", token);
}

bool isMatchingOperatorFunction(const FunctionDescription& function, const std::vector<Unit>& params)
{
    if (params.size() == 3 && params[1].isArrayWithFixedDim())
        if (isSameParamsType(function, { params[0], params[1] }))
            if (function.symbols.at(function.parameters[1]).isArrayWithFixedDim())
                return true;

    return isSameParamsType(function, params);
}

std::pair<std::string, FunctionDescription> DescriptionFinder::getCustomOperatorFunctionString(
    const Token& token,
    const std::string& binaryOperatorName,
    const std::vector<Unit>& params)
{
    for (auto functionName : workingModule->definedFunctions)
    {
        const std::string unmangleFunctionName = unMangleString(functionName, workingModule->name);

        if (unmangleFunctionName.rfind(binaryOperatorName, 0) != 0)
            continue;

        const FunctionDescription function = workingModule->functions.at(functionName);

        if (!isMatchingOperatorFunction(function, params))
            continue;

        return std::pair<std::string, FunctionDescription>(functionName, function);
    }

    console.operatorError(token, params[0], params[1]);
}

std::pair<std::string, FunctionDescription> DescriptionFinder::getCustomOperatorFunction(
    const Token& binaryOperator,
    const std::vector<Unit>& params)
{
    const std::string binaryOperatorName = binaryOperator.toFunctionNameString();

    return getCustomOperatorFunctionString(binaryOperator, binaryOperatorName, params);
}

std::pair<std::string, FunctionDescription> DescriptionFinder::getSubscriptOperatorFunction(
    const Token& token,
    const Unit& unit,
    const std::vector<Unit>& params)
{
    return getCustomOperatorFunctionString(token, "subscript", params);
}

bool DescriptionFinder::isAllNamesStructFields(const std::vector<Token>& nameTokens, const Unit& structUnit)
{
    const ModuleDescription unitModule = modulesMap.at(structUnit.moduleName());
    const StructDescription structDescription = unitModule.structures.at(structUnit.dtypeName());

    for (auto& nameToken : nameTokens)
        if (!structDescription.fieldExists(nameToken.toString()))
            return false;

    return true;
}
