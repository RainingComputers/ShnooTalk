#include "../pathchk.hpp"

#include "DescriptionFinder.hpp"

using namespace icode;

DescriptionFinder::DescriptionFinder(ModuleDescription& rootModule,
                                     StringModulesMap& modulesMap,
                                     Console& console,
                                     UnitBuilder& unitBuilder)
  : rootModule(rootModule)
  , modulesMap(modulesMap)
  , console(console)
  , unitBuilder(unitBuilder)
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
    return &modulesMap.at(unit.second.moduleName);
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

    returnValue = unitBuilder.unitPairFromTypeDescription(typeDescription, nameToken.toString());
    return true;
}

bool DescriptionFinder::getGlobal(const Token& nameToken, Unit& returnValue)
{
    TypeDescription typeDescription;

    if (!rootModule.getGlobal(nameToken.toString(), typeDescription))
        return false;

    returnValue = unitBuilder.unitPairFromTypeDescription(typeDescription, nameToken.toString());
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

bool DescriptionFinder::getDefine(const Token& nameToken, Unit& returnValue)
{
    DefineDescription defineDescription;

    if (!workingModule->getDefineDescription(nameToken.toString(), defineDescription))
        if (!rootModule.getDefineDescription(nameToken.toString(), defineDescription))
            return false;

    returnValue = unitBuilder.unitFromDefineDescription(defineDescription);
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

    if (getDefine(nameToken, unit))
        return unit;

    console.compileErrorOnToken("Symbol does not exist", nameToken);
}



int DescriptionFinder::getDataTypeSizeFromToken(const Token& nameToken)
{
    const std::string& dataTypeString = nameToken.toString();

    DataType dtype = workingModule->dataTypeFromString(dataTypeString);
    if (dtype != STRUCT)
        return getDataTypeSize(dtype);

    StructDescription structDescription;
    if (workingModule->getStruct(dataTypeString, structDescription))
        return structDescription.size;

    Unit unit = getUnitFromToken(nameToken);
    return unit.second.size;
}

FunctionDescription DescriptionFinder::getFunction(const Token& nameToken)
{
    FunctionDescription functionDescription;

    if ((*workingModule).getFunction(nameToken.toString(), functionDescription))
        return functionDescription;

    if (rootModule.getFunction(nameToken.toString(), functionDescription))
        return functionDescription;

    console.compileErrorOnToken("Function does not exist", nameToken);
}

std::vector<Unit> DescriptionFinder::getFormalParameters(const FunctionDescription& function)
{
    std::vector<Unit> formalParameters;

    for (const std::string& parameter : function.parameters)
    {
        TypeDescription paramType = function.symbols.at(parameter);
        formalParameters.push_back(unitBuilder.unitPairFromTypeDescription(paramType, parameter));
    }

    return formalParameters;
}

void DescriptionFinder::createFrom(const Token& moduleNameToken, const Token& symbolNameToken)
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

    else if ((*externalModule).getFunction(symbolString, functionDescription))
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