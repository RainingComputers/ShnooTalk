#include "NameMangle.hpp"

#include "IRBuilder.hpp"

IRBuilder::IRBuilder(icode::TargetEnums& target,
                     icode::StringModulesMap& modulesMap,
                     const std::string& fileName,
                     Console& console)

  : rootModule(modulesMap[fileName])
  , console(console)
  , moduleBuilder(modulesMap[fileName], modulesMap, console)
  , unitBuilder(modulesMap[fileName], opBuilder)
  , descriptionFinder(modulesMap[fileName], modulesMap, unitBuilder, console)
  , functionBuilder(modulesMap, opBuilder, unitBuilder, console)
{
    workingFunction = nullptr;
    workingModule = &rootModule;

    rootModule.name = fileName;
    rootModule.initializeTarget(target);

    setWorkingModule(workingModule);
}

void IRBuilder::resetWorkingModule()
{
    workingModule = &rootModule;
    moduleBuilder.setWorkingModule(&rootModule);
    descriptionFinder.setWorkingModule(&rootModule);
    unitBuilder.setWorkingModule(&rootModule);
}

void IRBuilder::setWorkingModule(icode::ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
    moduleBuilder.setWorkingModule(moduleDescription);
    descriptionFinder.setWorkingModule(moduleDescription);
    unitBuilder.setWorkingModule(moduleDescription);
}

void IRBuilder::pushWorkingModule()
{
    moduleDescriptionStack.push_back(workingModule);
}

void IRBuilder::popWorkingModule()
{
    setWorkingModule(moduleDescriptionStack.back());
    moduleDescriptionStack.pop_back();
}

void IRBuilder::setWorkingFunction(const Token& functionNameToken)
{
    std::string mangledFunctionName = nameMangle(functionNameToken, rootModule.name);

    workingFunction = &rootModule.functions[mangledFunctionName];
    functionBuilder.setWorkingFunction(workingFunction);
    descriptionFinder.setWorkingFunction(workingFunction);
}
