#include <algorithm>

#include "NameMangle.hpp"

#include "ModuleBuilder.hpp"

using namespace icode;

ModuleBuilder::ModuleBuilder(ModuleDescription& rootModule, StringModulesMap& modulesMap, Console& console)
    : rootModule(rootModule)
    , modulesMap(modulesMap)
    , console(console)
{
}

void ModuleBuilder::setWorkingModule(icode::ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

void ModuleBuilder::registerIncompleteType(const Token& typeName)
{
    rootModule.incompleteTypes[typeName.toString()] = rootModule.name;
}

DataType ModuleBuilder::tokenToDataType(const Token& token)
{
    const std::string& tokenString = token.toString();

    if (workingModule->enumTypeExists(tokenString))
        return ENUM;

    return stringToDataType(tokenString);
}

TypeDescription ModuleBuilder::createVoidTypeDescription()
{
    TypeDescription voidTypeDescription;

    voidTypeDescription.dtype = icode::VOID;
    voidTypeDescription.dtypeName = dataTypeToString(icode::VOID);
    voidTypeDescription.dtypeSize = 0;
    voidTypeDescription.size = 0;
    voidTypeDescription.offset = 0;
    voidTypeDescription.properties = 0;
    voidTypeDescription.moduleName = rootModule.name;

    return voidTypeDescription;
}

TypeDescription constructType(DataType dtype,
                              const std::string& dtypeName,
                              int dtypeSize,
                              const std::string& moduleName)
{
    TypeDescription typeDescription;

    typeDescription.dtype = dtype;
    typeDescription.dtypeName = dtypeName;
    typeDescription.dtypeSize = dtypeSize;
    typeDescription.size = dtypeSize;
    typeDescription.offset = 0;
    typeDescription.properties = 0;
    typeDescription.moduleName = moduleName;

    return typeDescription;
}

TypeInformation ModuleBuilder::getTypeInformation(const Token& dataTypeToken, DataType dtype)
{
    const std::string dataTypeName = dataTypeToken.toString();

    if (dtype != icode::STRUCT)
        return TypeInformation{ getDataTypeSize(dtype), workingModule->name, false };

    icode::StructDescription structDesc;
    if (workingModule->getStruct(dataTypeName, structDesc))
        return TypeInformation{ structDesc.size, structDesc.moduleName, false };

    std::string incompleteTypeModuleName;
    if (workingModule->getIncompleteTypeModule(dataTypeName, incompleteTypeModuleName))
        return TypeInformation{ 0, incompleteTypeModuleName, true };

    console.compileErrorOnToken("Type does not exist", dataTypeToken);
}

TypeDescription ModuleBuilder::createTypeDescription(const Token& dataTypeToken)
{
    icode::DataType dtype = tokenToDataType(dataTypeToken);

    TypeInformation typeInfo = getTypeInformation(dataTypeToken, dtype);

    TypeDescription type = constructType(dtype, dataTypeToken.toString(), typeInfo.dtypeSize, typeInfo.moduleName);

    if (typeInfo.incompleteType)
        type.becomeIncompleteType();

    return type;
}

void ModuleBuilder::createIntDefine(const Token& nameToken, int value)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.intDefines[nameToken.toString()] = value;
}

void ModuleBuilder::createFloatDefine(const Token& nameToken, float value)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.floatDefines[nameToken.toString()] = value;
}

std::string unescapedStringFromTokens(const std::vector<Token>& tokens)
{
    std::string str;

    for (const Token& tok : tokens)
        str += tok.toUnescapedString();

    str += '\0';

    return str;
}

std::string ModuleBuilder::createMultilineStringData(const std::vector<Token>& tokens)
{
    /* Check if this string has already been defined, if yes return the key for that,
        else create a new key (across all modules) */
    std::string str = unescapedStringFromTokens(tokens);

    for (auto modulesMapItem : modulesMap)
    {
        auto result = std::find_if(modulesMapItem.second.stringsData.begin(),
                                   modulesMapItem.second.stringsData.end(),
                                   [str](const auto& mapItem) { return mapItem.second == str; });

        if (result != modulesMapItem.second.stringsData.end())
        {
            const std::string key = result->first;
            rootModule.stringsDataCharCounts[key] = modulesMapItem.second.stringsDataCharCounts[key];
            return key;
        }
    }

    std::string key = lineColNameMangle(tokens[0], rootModule.name);
    rootModule.stringsData[key] = str;
    rootModule.stringsDataCharCounts[key] = str.size();

    return key;
}

std::string ModuleBuilder::createStringData(const Token& stringToken)
{
    const std::vector<Token> stringTokens = { stringToken };
    return createMultilineStringData(stringTokens);
}

void ModuleBuilder::createStringDefine(const Token& nameToken, const Token& valueToken)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    std::string key = createStringData(valueToken);
    rootModule.stringDefines[nameToken.toString()] = key;
}

void ModuleBuilder::createEnumType(const Token& nameToken)
{
    const std::string enumTypeName = nameToken.toString();

    if (rootModule.symbolExists(enumTypeName))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.definedEnumsTypes.push_back(enumTypeName);
}

void ModuleBuilder::createEnum(const Token& nameToken, const std::vector<Token>& enums)
{
    createEnumType(nameToken);

    for (size_t i = 0; i < enums.size(); i += 1)
    {
        const std::string enumName = enums[i].toString();

        if (rootModule.symbolExists(enumName))
            console.compileErrorOnToken("Symbol already exists", enums[i]);

        rootModule.enums[enumName] = { nameToken.toString(), long(i), rootModule.name };
    }
}

FunctionDescription ModuleBuilder::createFunctionDescription(const icode::TypeDescription& returnType,
                                                             const std::vector<Token>& paramNames,
                                                             std::vector<icode::TypeDescription>& paramTypes,
                                                             const std::string& moduleName)
{
    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnType = returnType;
    functionDescription.moduleName = moduleName;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        if (rootModule.symbolExists(paramNames[i].toString()))
            console.compileErrorOnToken("Symbol already exists", paramNames[i]);

        paramTypes[i].setProperty(IS_PARAM);
        paramTypes[i].setProperty(IS_LOCAL);

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    return functionDescription;
}

bool ModuleBuilder::isValidDeconstructor(const icode::FunctionDescription& function)
{
    if (!function.isVoid())
        return false;

    if (function.numParameters() != 1)
        return false;

    const TypeDescription symbolType = function.symbols.at(function.parameters[0]);

    if (rootModule.name != symbolType.moduleName)
        return false;

    return true;
}

void ModuleBuilder::createFunction(const Token& nameToken,
                                   const icode::TypeDescription& returnType,
                                   const std::vector<Token>& paramNames,
                                   std::vector<icode::TypeDescription>& paramTypes)
{
    std::string mangledFunctionName = nameMangle(nameToken, rootModule.name);

    if (rootModule.symbolExists(mangledFunctionName) || rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    const icode::FunctionDescription function =
        createFunctionDescription(returnType, paramNames, paramTypes, rootModule.name);

    if (nameToken.toString() == "deconstructor")
        if (!isValidDeconstructor(function))
            console.compileErrorOnToken("Invalid deconstructor function", nameToken);

    rootModule.functions[mangledFunctionName] = function;

    rootModule.definedFunctions.push_back(mangledFunctionName);
}

void ModuleBuilder::createFunctionExternC(const Token& nameToken,
                                          const icode::TypeDescription& returnType,
                                          const std::vector<Token>& paramNames,
                                          std::vector<icode::TypeDescription>& paramTypes)
{
    const std::string functionName = nameToken.toString();

    if (rootModule.symbolExists(functionName))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.functions[functionName] =
        createFunctionDescription(returnType, paramNames, paramTypes, rootModule.name);

    rootModule.definedFunctions.push_back(functionName);
}

void ModuleBuilder::createExternFunction(const Token& nameToken,
                                         const icode::TypeDescription& returnType,
                                         const std::vector<Token>& paramNames,
                                         std::vector<icode::TypeDescription>& paramTypes)
{
    const std::string externFunctionName = nameToken.toString();

    if (rootModule.symbolExists(externFunctionName))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.externFunctions[externFunctionName] =
        createFunctionDescription(returnType, paramNames, paramTypes, rootModule.name);
}

void ModuleBuilder::createExternFunctionModule(const Token& nameToken,
                                               const icode::TypeDescription& returnType,
                                               const std::vector<Token>& paramNames,
                                               std::vector<icode::TypeDescription>& paramTypes,
                                               const Token& moduleNameToken)
{
    const std::string functionModuleName = moduleNameToken.toUnescapedString();
    const std::string mangledFunctionName = nameMangle(nameToken, functionModuleName);
    const std::string mangledFunctionNameRoot = nameMangle(nameToken, rootModule.name);
    const std::string externFunctionName = nameToken.toString();

    if (rootModule.symbolExists(mangledFunctionName) || rootModule.symbolExists(mangledFunctionNameRoot) ||
        rootModule.symbolExists(externFunctionName))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    rootModule.externFunctions[mangledFunctionName] =
        createFunctionDescription(returnType, paramNames, paramTypes, functionModuleName);

    rootModule.incompleteFunctions[externFunctionName] = functionModuleName;
}

void ModuleBuilder::createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription)
{
    std::string mangledGlobalName = nameMangle(globalNameToken, rootModule.name);

    if (rootModule.symbolExists(mangledGlobalName) || rootModule.symbolExists(globalNameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", globalNameToken);

    typeDescription.setProperty(IS_GLOBAL);

    rootModule.globals[mangledGlobalName] = typeDescription;
}

icode::StructDescription ModuleBuilder::createEmptyStructDescription()
{
    icode::StructDescription structDescription;
    structDescription.moduleName = rootModule.name;
    structDescription.size = 0;

    return structDescription;
}

void ModuleBuilder::createStruct(const Token& nameToken,
                                 const std::vector<Token>& fieldNameTokens,
                                 const std::vector<icode::TypeDescription>& fieldTypes)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    icode::StructDescription structDescription = createEmptyStructDescription();

    for (size_t i = 0; i < fieldNameTokens.size(); i++)
    {
        const std::string fieldName = fieldNameTokens[i].toString();

        if (structDescription.fieldExists(fieldName))
            console.compileErrorOnToken("Field already exists", fieldNameTokens[i]);

        icode::TypeDescription field = fieldTypes[i];
        field.offset = structDescription.size;

        if (field.isPointer())
            structDescription.size += getDataTypeSize(icode::AUTO_INT);
        else
            structDescription.size += field.size;

        structDescription.structFields[fieldName] = field;
        structDescription.fieldNames.push_back(fieldName);
    }

    rootModule.structures[nameToken.toString()] = structDescription;
}

void ModuleBuilder::createUseNoAlias(const Token& pathToken)
{
    const std::string path = pathToken.toUnescapedString();

    if (!rootModule.useExists(path))
        rootModule.uses.push_back(path);
}

void ModuleBuilder::createUse(const Token& pathToken, const Token& aliasToken)
{
    const std::string path = pathToken.toUnescapedString();
    const std::string alias = aliasToken.toString();

    if (rootModule.useExists(path))
        console.compileErrorOnToken("Duplicate imports", pathToken);

    if (rootModule.aliasExists(alias))
        console.compileErrorOnToken("Symbol already exists", aliasToken);

    createUseNoAlias(pathToken);
    rootModule.aliases[alias] = path;
}

void ModuleBuilder::createFrom(const std::string& moduleName, const Token& symbolNameToken)
{
    /* Used to store return values */
    StructDescription structDescReturnValue;
    FunctionDescription funcDescReturnValue;
    long intDefineReturnValue;
    double floatDefineReturnValue;
    EnumDescription enumReturnValue;
    std::string stringDataKeyReturnValue;
    std::string importModuleNameReturnValue;

    ModuleDescription* externalModule = &(modulesMap.at(moduleName));

    /* Import symbol */

    const std::string symbolString = symbolNameToken.toString();
    const std::string mangledSymbolString = nameMangle(symbolNameToken, externalModule->name);

    if (rootModule.symbolExists(symbolString))
        console.compileErrorOnToken("Symbol already exists", symbolNameToken);

    if (externalModule->getStruct(symbolString, structDescReturnValue))
        rootModule.structures[symbolString] = structDescReturnValue;

    else if (externalModule->getExternFunction(symbolString, funcDescReturnValue))
        rootModule.externFunctions[symbolString] = funcDescReturnValue;

    else if (externalModule->getFunction(mangledSymbolString, funcDescReturnValue))
    {
        funcDescReturnValue.icodeTable.clear();
        rootModule.externFunctions[symbolString] = funcDescReturnValue;
    }

    else if (externalModule->getIntDefine(symbolString, intDefineReturnValue))
        rootModule.intDefines[symbolString] = intDefineReturnValue;

    else if (externalModule->getFloatDefine(symbolString, floatDefineReturnValue))
        rootModule.floatDefines[symbolString] = floatDefineReturnValue;

    else if (externalModule->getStringDefine(symbolString, stringDataKeyReturnValue))
    {
        rootModule.stringDefines[symbolString] = stringDataKeyReturnValue;
        rootModule.stringsDataCharCounts[stringDataKeyReturnValue] =
            externalModule->stringsDataCharCounts[stringDataKeyReturnValue];
    }

    else if (externalModule->getEnum(symbolString, enumReturnValue))
        rootModule.enums[symbolString] = enumReturnValue;

    else if (externalModule->getModuleNameFromAlias(symbolString, importModuleNameReturnValue))
    {
        if (rootModule.aliasExists(symbolString))
            console.compileErrorOnToken("Alias already exists", symbolNameToken);

        rootModule.uses.push_back(importModuleNameReturnValue);
        rootModule.aliases[symbolString] = importModuleNameReturnValue;
    }

    else
        console.compileErrorOnToken("Symbol does not exist", symbolNameToken);
}

void ModuleBuilder::createDirectFrom(const Token& pathToken, const Token& symbolNameToken)
{
    createUseNoAlias(pathToken);
    createFrom(pathToken.toUnescapedString(), symbolNameToken);
}

void ModuleBuilder::createAliasFrom(const Token& aliasToken, const Token& symbolNameToken)
{
    /* Get module description from alias */
    std::string moduleName;
    if (!rootModule.getModuleNameFromAlias(aliasToken.toString(), moduleName))
        console.compileErrorOnToken("Use does not exist", aliasToken);

    createFrom(moduleName, symbolNameToken);
}
