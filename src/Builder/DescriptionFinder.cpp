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

    returnValue = unitBuilder.unitPairFromTypeDescription(typeDescription, nameToken);
    return true;
}

bool DescriptionFinder::getGlobal(const Token& nameToken, Unit& returnValue)
{
    TypeDescription typeDescription;

    if (!rootModule.getGlobal(nameToken.toString(), typeDescription))
        return false;

    returnValue = unitBuilder.unitPairFromTypeDescription(typeDescription, nameToken);
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

void DescriptionFinder::createUse(const Token& nameToken)
{
    bool isFile = pathchk::file_exists(nameToken.toString() + ".uhll");
    bool isFolder = pathchk::dir_exists(nameToken.toString());

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

void DescriptionFinder::createFrom(const Token& moduleNameToken, const Token& symbolNameToken)
{
    icode::StructDescription sturctDescription;
    icode::FunctionDescription functionDescription;
    icode::DefineDescription defineDescription;
    int enumValue;

    if (!workingModule->useExists(moduleNameToken.toString()))
        console.compileErrorOnToken("Module not imported", moduleNameToken);

    icode::ModuleDescription* externalModule = &modulesMap[moduleNameToken.toString()];

    const std::string& symbolString = symbolNameToken.toString();

    if (workingModule->symbolExists(symbolString))
        console.compileErrorOnToken("Symbol already defined in current module", symbolNameToken);

    if ((*externalModule).getStruct(symbolString, sturctDescription))
        workingModule->structures[symbolString] = sturctDescription;

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