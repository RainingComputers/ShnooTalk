#include "DescriptionFinder.hpp"

using namespace icode;

DescriptionFinder::DescriptionFinder(ModuleDescription& rootModule, Console& console, UnitBuilder& unitBuilder)
  : rootModule(rootModule)
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

ModuleDescription* DescriptionFinder::getModuleFromToken(const Token& moduleNameToken,
                                                         StringModulesMap& modulesMap)
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