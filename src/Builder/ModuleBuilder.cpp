#include <algorithm>
#include <filesystem>

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

std::pair<int, std::string> ModuleBuilder::getSizeAndModuleName(const Token& dataTypeToken, DataType dtype)
{
    if (dtype != icode::STRUCT)
        return std::pair<int, std::string>(getDataTypeSize(dtype), workingModule->name);

    icode::StructDescription structDesc;
    if (!workingModule->getStruct(dataTypeToken.toString(), structDesc))
        console.compileErrorOnToken("Data type does not exist", dataTypeToken);

    return std::pair<int, std::string>(structDesc.size, structDesc.moduleName);
}

TypeDescription ModuleBuilder::createTypeDescription(const Token& dataTypeToken)
{
    icode::DataType dtype = stringToDataType(dataTypeToken.toString());

    std::pair<int, std::string> sizeAndModuleName = getSizeAndModuleName(dataTypeToken, dtype);

    TypeDescription typeDescription;

    typeDescription.dtype = dtype;
    typeDescription.dtypeName = dataTypeToken.toString();
    typeDescription.dtypeSize = sizeAndModuleName.first;
    typeDescription.size = typeDescription.dtypeSize;
    typeDescription.offset = 0;
    typeDescription.properties = 0;
    typeDescription.moduleName = sizeAndModuleName.second;

    return typeDescription;
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

std::string ModuleBuilder::createStringData(const Token& stringToken)
{
    /* Check if this string has already been defined, if yes return the key for that,
        else create a new key (across all modules) */

    std::string str = stringToken.toUnescapedString() + '\0';

    for (auto modulesMapItem : modulesMap)
    {
        auto result = std::find_if(modulesMapItem.second.stringsData.begin(),
                                   modulesMapItem.second.stringsData.end(),
                                   [str](const auto& mapItem) { return mapItem.second == str; });

        if (result != modulesMapItem.second.stringsData.end())
        {
            const std::string& key = result->first;
            rootModule.stringsDataCharCounts[key] = modulesMapItem.second.stringsDataCharCounts[key];
            return key;
        }
    }

    std::string key = lineColNameMangle(stringToken, rootModule.name);
    rootModule.stringsData[key] = str;
    rootModule.stringsDataCharCounts[key] = str.size();

    return key;
}

void ModuleBuilder::createStringDefine(const Token& nameToken, const Token& valueToken)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    std::string key = createStringData(valueToken);
    rootModule.stringDefines[nameToken.toString()] = key;
}

void ModuleBuilder::createEnum(const std::vector<Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (rootModule.symbolExists(enums[i].toString()))
            console.compileErrorOnToken("Symbol already defined", enums[i]);

        rootModule.enumerations[enums[i].toString()] = i;
    }
}

FunctionDescription ModuleBuilder::createFunctionDescription(const icode::TypeDescription& returnType,
                                                             const std::vector<Token>& paramNames,
                                                             std::vector<icode::TypeDescription>& paramTypes)
{
    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnType = returnType;
    functionDescription.moduleName = rootModule.name;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        if (rootModule.symbolExists(paramNames[i].toString()))
            console.compileErrorOnToken("Symbol already defined", paramNames[i]);

        paramTypes[i].setProperty(IS_PARAM);

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    return functionDescription;
}

void ModuleBuilder::createFunction(const Token& nameToken,
                                   const icode::TypeDescription& returnType,
                                   const std::vector<Token>& paramNames,
                                   std::vector<icode::TypeDescription>& paramTypes)
{
    std::string mangledFunctionName = nameMangle(nameToken, rootModule.name);

    if (rootModule.symbolExists(mangledFunctionName) || rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    rootModule.functions[mangledFunctionName] = createFunctionDescription(returnType, paramNames, paramTypes);
}

void ModuleBuilder::createExternFunction(const Token& nameToken,
                                         const icode::TypeDescription& returnType,
                                         const std::vector<Token>& paramNames,
                                         std::vector<icode::TypeDescription>& paramTypes)
{
    const std::string& externFunctionName = nameToken.toString();

    if (rootModule.symbolExists(externFunctionName))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    rootModule.externFunctions[externFunctionName] = createFunctionDescription(returnType, paramNames, paramTypes);
}

void ModuleBuilder::createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription)
{
    std::string mangledGlobalName = nameMangle(globalNameToken, rootModule.name);

    if (rootModule.symbolExists(mangledGlobalName) || rootModule.symbolExists(globalNameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", globalNameToken);

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
                                 const std::vector<Token>& fieldNames,
                                 const std::vector<icode::TypeDescription>& fieldTypes)
{
    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::StructDescription structDescription = createEmptyStructDescription();

    for (size_t i = 0; i < fieldNames.size(); i++)
    {
        if (rootModule.symbolExists(fieldNames[i].toString()))
            console.compileErrorOnToken("Symbol already defined", fieldNames[i]);

        if (structDescription.fieldExists(fieldNames[i].toString()))
            console.compileErrorOnToken("Field already defined", fieldNames[i]);

        icode::TypeDescription field = fieldTypes[i];
        field.offset = structDescription.size;

        structDescription.size += field.size;

        structDescription.structFields[fieldNames[i].toString()] = field;
    }

    rootModule.structures[nameToken.toString()] = structDescription;
}

void ModuleBuilder::createUse(const Token& pathToken, const Token& aliasToken)
{
    const std::string& path = pathToken.toUnescapedString();
    const std::string& alias = aliasToken.toString();

    if (!std::filesystem::exists(path))
        console.compileErrorOnToken("File does not exist", pathToken);

    if (rootModule.useExists(path))
        console.compileErrorOnToken("Multiple imports detected", pathToken);

    if (rootModule.aliasExists(alias))
        console.compileErrorOnToken("Symbol already defined", aliasToken);

    rootModule.uses.push_back(path);
    rootModule.aliases[alias] = path;
}

void ModuleBuilder::createFrom(const Token& aliasToken, const Token& symbolNameToken)
{
    /* Used to store return values */
    StructDescription structDescReturnValue;
    FunctionDescription funcDescReturnValue;
    long intDefineReturnValue;
    double floatDefineReturnValue;
    int enumReturnValue;
    std::string stringDataKeyReturnValue;
    std::string importModuleNameReturnValue;

    /* Get module description from alias */
    std::string moduleName;
    if (!rootModule.getModuleNameFromAlias(aliasToken.toString(), moduleName))
        console.compileErrorOnToken("Use does not exist", aliasToken);

    ModuleDescription* externalModule = &(modulesMap.at(moduleName));

    /* Import symbol */

    const std::string& symbolString = symbolNameToken.toString();
    const std::string& mangledSymbolString = nameMangle(symbolString, externalModule->name);

    if (rootModule.symbolExists(symbolString))
        console.compileErrorOnToken("Symbol already defined", symbolNameToken);

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
        rootModule.enumerations[symbolString] = enumReturnValue;

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