#include "../FileSystem/FileSystem.hpp"
#include "NameMangle.hpp"

#include "ModuleBuilder.hpp"

using namespace icode;

ModuleBuilder::ModuleBuilder(StringModulesMap& modulesMap, Console& console)
  : modulesMap(modulesMap)
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
    voidTypeDescription.moduleName = workingModule->name;

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

icode::DefineDescription defineDescriptionFromToken(const Token& valueToken)
{
    if (valueToken.getType() == token::INT_LITERAL)
        return icode::createIntDefineDescription(valueToken.toInt(), icode::AUTO_INT);

    return icode::createFloatDefineDescription(valueToken.toFloat(), icode::AUTO_FLOAT);
}

void ModuleBuilder::createDefine(const Token& nameToken, const Token& valueToken)
{
    if (workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    workingModule->defines[nameToken.toString()] = defineDescriptionFromToken(valueToken);
}

void ModuleBuilder::createEnum(const std::vector<Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (workingModule->symbolExists(enums[i].toString()))
            console.compileErrorOnToken("Symbol already defined", enums[i]);

        workingModule->enumerations[enums[i].toString()] = i;
    }
}

void ModuleBuilder::createFunctionDescription(const Token& nameToken,
                                              const icode::TypeDescription& returnType,
                                              const std::vector<Token>& paramNames,
                                              std::vector<icode::TypeDescription>& paramTypes)
{
    std::string mangledFunctionName = nameMangle(nameToken, workingModule->name);

    if (workingModule->symbolExists(mangledFunctionName) || workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnType = returnType;
    functionDescription.moduleName = workingModule->name;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        if (workingModule->symbolExists(paramNames[i].toString()))
            console.compileErrorOnToken("Symbol already defined", paramNames[i]);

        paramTypes[i].setProperty(IS_PARAM);

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    workingModule->functions[mangledFunctionName] = functionDescription;
}

void ModuleBuilder::createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription)
{
    std::string mangledGlobalName = nameMangle(globalNameToken, workingModule->name);

    if (workingModule->symbolExists(mangledGlobalName) || workingModule->symbolExists(globalNameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", globalNameToken);

    typeDescription.setProperty(IS_GLOBAL);

    workingModule->globals[mangledGlobalName] = typeDescription;
}

icode::StructDescription ModuleBuilder::createEmptyStructDescription()
{
    icode::StructDescription structDescription;
    structDescription.moduleName = workingModule->name;
    structDescription.size = 0;

    return structDescription;
}

void ModuleBuilder::createStructDescription(const Token& nameToken,
                                            const std::vector<Token>& fieldNames,
                                            const std::vector<icode::TypeDescription>& fieldTypes)
{
    if (workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::StructDescription structDescription = createEmptyStructDescription();

    for (size_t i = 0; i < fieldNames.size(); i++)
    {
        if (workingModule->symbolExists(fieldNames[i].toString()))
            console.compileErrorOnToken("Symbol already defined", fieldNames[i]);

        if (structDescription.fieldExists(fieldNames[i].toString()))
            console.compileErrorOnToken("Field already defined", fieldNames[i]);

        icode::TypeDescription field = fieldTypes[i];
        field.offset = structDescription.size;

        structDescription.size += field.size;

        structDescription.structFields[fieldNames[i].toString()] = field;
    }

    workingModule->structures[nameToken.toString()] = structDescription;
}

void ModuleBuilder::createUse(const Token& nameToken)
{
    bool isFile = fs::fileExists(nameToken.toString() + ".uhll");
    bool isFolder = fs::directoryExists(nameToken.toString());

    if (!(isFile || isFolder))
        console.compileErrorOnToken("Module or Package does not exist", nameToken);

    if (isFile && isFolder)
        console.compileErrorOnToken("Module and Package exists with same name", nameToken);

    if (workingModule->useExists(nameToken.toString()))
        console.compileErrorOnToken("Multiple imports detected", nameToken);

    if (workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Name conflict, symbol already exists", nameToken);

    if (workingModule->name == nameToken.toString())
        console.compileErrorOnToken("Self import not allowed", nameToken);

    workingModule->uses.push_back(nameToken.toString());
}

void ModuleBuilder::createFrom(const Token& moduleNameToken, const Token& symbolNameToken)
{
    StructDescription structDescription;
    FunctionDescription functionDescription;
    DefineDescription defineDescription;
    int enumValue;

    if (!workingModule->useExists(moduleNameToken.toString()))
        console.compileErrorOnToken("Module not imported", moduleNameToken);

    ModuleDescription* externalModule = &modulesMap[moduleNameToken.toString()];

    const std::string& symbolString = symbolNameToken.toString();

    if (workingModule->symbolExists(symbolString))
        console.compileErrorOnToken("Symbol already defined in current module", symbolNameToken);

    if ((*externalModule).getStruct(symbolString, structDescription))
        workingModule->structures[symbolString] = structDescription;

    else if ((*externalModule).getFunction(nameMangle(symbolString, externalModule->name), functionDescription))
        console.compileErrorOnToken("Cannot import functions", symbolNameToken);

    else if ((*externalModule).getDefineDescription(symbolString, defineDescription))
        workingModule->defines[symbolString] = defineDescription;

    else if ((*externalModule).getEnum(symbolString, enumValue))
        workingModule->enumerations[symbolString] = enumValue;

    else if ((*externalModule).useExists(symbolString))
        workingModule->uses.push_back(symbolString);

    else
        console.compileErrorOnToken("Symbol does not exist", symbolNameToken);
}