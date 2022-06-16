#include "NameMangle.hpp"
#include "TypeCheck.hpp"

#include "DescriptionFinder.hpp"

using namespace icode;

Finder::Finder(ModuleDescription& rootModule, StringModulesMap& modulesMap, UnitBuilder& unitBuilder, Console& console)
    : rootModule(rootModule)
    , modulesMap(modulesMap)
    , unitBuilder(unitBuilder)
    , console(console)
{
}

void Finder::setWorkingModule(ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

void Finder::setWorkingFunction(FunctionDescription* functionDescription)
{
    workingFunction = functionDescription;
}

ModuleDescription* Finder::getModuleFromType(const TypeDescription& type)
{
    return &modulesMap.at(type.moduleName);
}

ModuleDescription* Finder::getModuleFromUnit(const Unit& unit)
{
    return &modulesMap.at(unit.moduleName());
}

ModuleDescription* Finder::getModuleFromToken(const Token& aliasToken)
{
    const std::string alias = aliasToken.toString();

    std::string moduleName;

    if (!workingModule->getModuleNameFromAlias(alias, moduleName))
        console.compileErrorOnToken("Use does not exist", aliasToken);

    return &modulesMap.at(moduleName);
}

StructDescription Finder::getStructDescFromType(const TypeDescription& type)
{
    return modulesMap.at(type.moduleName).structures.at(type.dtypeName);
}

StructDescription Finder::getStructDescFromUnit(const Unit& unit)
{
    return getStructDescFromType(unit.type());
}

std::vector<TypeDescription> Finder::getFieldTypes(const TypeDescription& type)
{
    const StructDescription structDescription = getStructDescFromType(type);

    std::vector<TypeDescription> fieldTypes;

    for (const std::string& fieldName : structDescription.fieldNames)
        fieldTypes.push_back(structDescription.structFields.at(fieldName));

    return fieldTypes;
}

std::vector<std::string> Finder::getFieldNames(const TypeDescription& type)
{
    return getStructDescFromType(type).fieldNames;
}

std::vector<TypeDescription> Finder::destructureStructType(const TypeDescription& type)
{
    const StructDescription structDescription = getStructDescFromType(type);

    std::vector<TypeDescription> types;

    for (const std::string& fieldName : structDescription.fieldNames)
        types.push_back(structDescription.structFields.at(fieldName));

    return types;
}

std::map<std::string, TypeDescription> Finder::destructureStructTypeMapped(const TypeDescription& type)
{
    const StructDescription structDescription = getStructDescFromType(type);

    std::map<std::string, TypeDescription> typeMap;

    for (const std::string& fieldName : structDescription.fieldNames)
        typeMap[fieldName] = structDescription.structFields.at(fieldName);

    return typeMap;
}

bool Finder::getLocal(const Token& nameToken, Unit& returnValue)
{
    TypeDescription typeDescription;

    if (!workingFunction->getSymbol(nameToken.toString(), typeDescription))
        return false;

    returnValue = unitBuilder.unitFromTypeDescription(typeDescription, nameToken.toString());
    return true;
}

bool Finder::getGlobal(const Token& nameToken, Unit& returnValue)
{
    std::string mangledGlobalName = nameMangle(nameToken, rootModule.name);

    TypeDescription typeDescription;

    if (!rootModule.getGlobal(mangledGlobalName, typeDescription))
        return false;

    returnValue = unitBuilder.unitFromTypeDescription(typeDescription, mangledGlobalName);
    return true;
}

bool Finder::getEnum(const Token& nameToken, Unit& returnValue)
{
    EnumDescription enumDescription;

    if (!workingModule->getEnum(nameToken.toString(), enumDescription))
        if (!rootModule.getEnum(nameToken.toString(), enumDescription))
            return false;

    returnValue = unitBuilder.unitFromEnum(enumDescription);
    return true;
}

bool Finder::getIntDefine(const Token& nameToken, Unit& returnValue)
{
    long defineValue;

    if (!workingModule->getIntDefine(nameToken.toString(), defineValue))
        if (!rootModule.getIntDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromIntLiteral(defineValue);
    return true;
}

bool Finder::getFloatDefine(const Token& nameToken, Unit& returnValue)
{
    double defineValue;

    if (!workingModule->getFloatDefine(nameToken.toString(), defineValue))
        if (!rootModule.getFloatDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromFloatLiteral(defineValue);
    return true;
}

bool Finder::getStringDefine(const Token& nameToken, Unit& returnValue)
{
    std::string defineValue;

    if (!workingModule->getStringDefine(nameToken.toString(), defineValue))
        if (!rootModule.getStringDefine(nameToken.toString(), defineValue))
            return false;

    returnValue = unitBuilder.unitFromStringDataKey(defineValue);
    return true;
}

Unit Finder::getUnitFromToken(const Token& nameToken)
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

FunctionDescription Finder::getFunction(const Token& nameToken)
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

    if (rootModule.getExternFunction(nameToken.toString(), functionDescription))
        return functionDescription;

    console.compileErrorOnToken("Function does not exist", nameToken);
}

std::vector<Unit> Finder::getFormalParameters(const FunctionDescription& function)
{
    std::vector<Unit> formalParameters;

    for (const std::string& parameterName : function.parameters)
    {
        TypeDescription paramType = function.getParamType(parameterName);
        formalParameters.push_back(unitBuilder.unitFromTypeDescription(paramType, parameterName));
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
        const TypeDescription formalParamType = function.getParamTypePos(i);

        if (!isSameTypeDescription(formalParamType, actualParamType))
            return false;
    }

    return true;
}

FunctionDescription Finder::getFunctionByParamTypes(const Token& token,
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

        return function;
    }

    console.compileErrorOnToken("Cannot find function with matching params", token);
}

bool isSameParamsTypeFixedDim(const FunctionDescription& function, const std::vector<Unit>& params)
{
    if (params.size() != 3)
        return false;

    if (!params[1].isArrayWithFixedDim())
        return false;

    if (!isSameParamsType(function, { params[0], params[1] }))
        return false;

    const TypeDescription secondFormalParam = function.getParamTypePos(1);

    return secondFormalParam.isArrayWithFixedDim();
}

FunctionDescription Finder::getCustomOperatorFunction(const std::string& binaryOperatorName,
                                                      const std::vector<Unit>& params,
                                                      const Token& errorToken)
{
    for (auto functionName : workingModule->definedFunctions)
    {
        const std::string unmangleFunctionName = unMangleString(functionName, workingModule->name);

        if (unmangleFunctionName.rfind(binaryOperatorName, 0) != 0)
            continue;

        const FunctionDescription function = workingModule->functions.at(functionName);

        if (isSameParamsType(function, params) || isSameParamsTypeFixedDim(function, params))
            return function;
    }

    console.operatorError(errorToken, params[0], params[1]);
}

bool Finder::isAllNamesStructFields(const std::vector<Token>& nameTokens, const TypeDescription& type)
{
    if (!type.isStruct())
        return false;

    const StructDescription structDescription = getStructDescFromType(type);

    for (auto& nameToken : nameTokens)
        if (!structDescription.fieldExists(nameToken.toString()))
            return false;

    return true;
}

bool Finder::deconstructorExists(const icode::TypeDescription& type)
{
    return getStructDescFromType(type).deconstructor != "";
}

std::string Finder::getMangledMethodName(const TypeDescription& type, const std::string& method)
{
    return nameMangleString(method, type.moduleName);
}

bool Finder::methodExists(const icode::TypeDescription& type, const std::string& method)
{
    const icode::ModuleDescription typeModule = modulesMap.at(type.moduleName);
    return typeModule.functionExists(getMangledMethodName(type, method));
}

FunctionDescription Finder::getMethod(const TypeDescription& type, const std::string& method)
{
    const std::string mangledFunctionName = getMangledMethodName(type, method);
    const icode::ModuleDescription typeModule = modulesMap.at(type.moduleName);

    return typeModule.functions.at(mangledFunctionName);
}

FunctionDescription Finder::getMethodFromUnit(const Unit& unit, const std::string& method, const Token& errorToken)
{
    const TypeDescription type = unit.type();
    const std::string mangledFunctionName = getMangledMethodName(type, method);
    icode::ModuleDescription typeModule = modulesMap.at(type.moduleName);

    icode::FunctionDescription function;
    if (!typeModule.getFunction(mangledFunctionName, function))
        console.compileErrorOnToken("Method " + method + " does not exist", errorToken);

    return function;
}
