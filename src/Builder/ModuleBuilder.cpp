#include "ModuleBuilder.hpp"

using namespace icode;

ModuleBuilder::ModuleBuilder(Console& console)
  : console(console)
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
    icode::DataType dtype = workingModule->dataTypeFromString(dataTypeToken.toString());

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
        return icode::createIntDefineDescription(valueToken.toInt(), icode::INT);

    return icode::createFloatDefineDescription(valueToken.toFloat(), icode::FLOAT);
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
    if (workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnDescription = returnType;
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

    workingModule->functions[nameToken.toString()] = functionDescription;
}

void ModuleBuilder::createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription)
{
    if (workingModule->symbolExists(globalNameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", globalNameToken);

    typeDescription.setProperty(IS_GLOBAL);

    workingModule->globals[globalNameToken.toString()] = typeDescription;
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
