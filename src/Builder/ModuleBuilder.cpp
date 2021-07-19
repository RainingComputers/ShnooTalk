#include <algorithm>

#include "../FileSystem/FileSystem.hpp"
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
            return result->first;
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

void ModuleBuilder::createFunctionDescription(const Token& nameToken,
                                              const icode::TypeDescription& returnType,
                                              const std::vector<Token>& paramNames,
                                              std::vector<icode::TypeDescription>& paramTypes)
{
    std::string mangledFunctionName = nameMangle(nameToken, rootModule.name);

    if (rootModule.symbolExists(mangledFunctionName) || rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

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

    rootModule.functions[mangledFunctionName] = functionDescription;
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

void ModuleBuilder::createStructDescription(const Token& nameToken,
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

void ModuleBuilder::createUse(const Token& nameToken)
{
    bool isFile = fs::fileExists(nameToken.toString() + ".uhll");
    bool isFolder = fs::directoryExists(nameToken.toString());

    if (!(isFile || isFolder))
        console.compileErrorOnToken("Module or Package does not exist", nameToken);

    if (isFile && isFolder)
        console.compileErrorOnToken("Module and Package exists with same name", nameToken);

    if (rootModule.useExists(nameToken.toString()))
        console.compileErrorOnToken("Multiple imports detected", nameToken);

    if (rootModule.symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Name conflict, symbol already exists", nameToken);

    if (rootModule.name == nameToken.toString())
        console.compileErrorOnToken("Self import not allowed", nameToken);

    rootModule.uses.push_back(nameToken.toString());
}

void ModuleBuilder::createFrom(const Token& moduleNameToken, const Token& symbolNameToken)
{
    StructDescription structDescription;
    FunctionDescription functionDescription;
    int intDefineValue;
    float floatDefineValue;
    int enumValue;
    std::string stringDataKey;

    if (!rootModule.useExists(moduleNameToken.toString()))
        console.compileErrorOnToken("Module not imported", moduleNameToken);

    ModuleDescription* externalModule = &modulesMap[moduleNameToken.toString()];

    const std::string& symbolString = symbolNameToken.toString();

    if (rootModule.symbolExists(symbolString))
        console.compileErrorOnToken("Symbol already defined in current module", symbolNameToken);

    if ((*externalModule).getStruct(symbolString, structDescription))
        rootModule.structures[symbolString] = structDescription;

    else if ((*externalModule).getFunction(nameMangle(symbolString, externalModule->name), functionDescription))
        console.compileErrorOnToken("Cannot import functions", symbolNameToken);

    else if ((*externalModule).getIntDefine(symbolString, intDefineValue))
        rootModule.intDefines[symbolString] = intDefineValue;

    else if ((*externalModule).getFloatDefine(symbolString, floatDefineValue))
        rootModule.floatDefines[symbolString] = floatDefineValue;

    else if ((*externalModule).getStringDefine(symbolString, stringDataKey))
    {
        rootModule.stringDefines[symbolString] = stringDataKey;
        rootModule.stringsDataCharCounts[stringDataKey] = (*externalModule).stringsDataCharCounts[stringDataKey];
    }

    else if ((*externalModule).getEnum(symbolString, enumValue))
        rootModule.enumerations[symbolString] = enumValue;

    else if ((*externalModule).useExists(symbolString))
        rootModule.uses.push_back(symbolString);

    else
        console.compileErrorOnToken("Symbol does not exist", symbolNameToken);
}