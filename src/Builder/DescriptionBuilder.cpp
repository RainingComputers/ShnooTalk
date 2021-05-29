#include "DescriptionBuilder.hpp"

using namespace icode;

DescriptionBuilder::DescriptionBuilder(Console& console)
  : console(console)
{
}

void DescriptionBuilder::setWorkingModule(icode::ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

ModuleDescription* DescriptionBuilder::getModuleFromToken(const token::Token& moduleNameToken,
                                                          StringModulesMap& modulesMap)
{
    const std::string& moduleName = moduleNameToken.toString();

    if (!workingModule->useExists(moduleName))
        console.compileErrorOnToken("Module does not exist", moduleNameToken);

    return &modulesMap.at(moduleName);
}

TypeDescription DescriptionBuilder::createVoidTypeDescription()
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

std::pair<int, std::string> DescriptionBuilder::getSizeAndModuleName(const token::Token& dataTypeToken, DataType dtype)
{
    if (dtype != icode::STRUCT)
        return std::pair<int, std::string>(getDataTypeSize(dtype), workingModule->name);

    icode::StructDescription structDesc;
    if (!workingModule->getStruct(dataTypeToken.toString(), structDesc))
        console.compileErrorOnToken("Symbol does not exist", dataTypeToken);

    return std::pair<int, std::string>(structDesc.size, structDesc.moduleName);
}

TypeDescription DescriptionBuilder::createTypeDescription(const token::Token& dataTypeToken)
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

TypeDescription DescriptionBuilder::createArrayTypeDescription(const TypeDescription& typeDescription,
                                                               std::vector<int>& dimensions)
{
    TypeDescription modifiedTypeDescription = typeDescription;

    for (int dim : dimensions)
    {
        modifiedTypeDescription.size *= dim;
        modifiedTypeDescription.dimensions.push_back(dim);
    }

    return modifiedTypeDescription;
}

icode::DefineDescription defineDescriptionFromToken(const token::Token& valueToken)
{
    if (valueToken.getType() == token::INT_LITERAL)
        return icode::createIntDefineDescription(valueToken.toInt(), icode::INT);

    return icode::createFloatDefineDescription(valueToken.toFloat(), icode::FLOAT);
}

void DescriptionBuilder::createDefine(const token::Token& nameToken, const token::Token& valueToken)
{
    if (workingModule->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already exists", nameToken);

    workingModule->defines[nameToken.toString()] = defineDescriptionFromToken(valueToken);
}

void DescriptionBuilder::createEnum(const std::vector<token::Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (workingModule->symbolExists(enums[i].toString()))
            console.compileErrorOnToken("Symbol already defined", enums[i]);

        workingModule->enumerations[enums[i].toString()] = i;
    }
}

void DescriptionBuilder::createFunctionDescription(const token::Token& nameToken,
                                                   const icode::TypeDescription& returnType,
                                                   const std::vector<token::Token>& paramNames,
                                                   const std::vector<icode::TypeDescription>& paramTypes)
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

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    workingModule->functions[nameToken.toString()] = functionDescription;
}

void DescriptionBuilder::createGlobal(const token::Token globalNameToken,
                                      const icode::TypeDescription& typeDescription)
{
    if (workingModule->symbolExists(globalNameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", globalNameToken);

    workingModule->globals[globalNameToken.toString()] = typeDescription;
}

icode::StructDescription DescriptionBuilder::createEmptyStructDescription()
{
    icode::StructDescription structDescription;
    structDescription.moduleName = workingModule->name;
    structDescription.size = 0;

    return structDescription;
}

void DescriptionBuilder::createStructDescription(const token::Token& nameToken,
                                                 const std::vector<token::Token>& fieldNames,
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
