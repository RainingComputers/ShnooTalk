#include "NameMangle.hpp"

#include "IRBuilder.hpp"

ShnooTalkIRBuilder::ShnooTalkIRBuilder(icode::TargetEnums& target,
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

void ShnooTalkIRBuilder::resetWorkingModule()
{
    workingModule = &rootModule;
    moduleBuilder.setWorkingModule(&rootModule);
    descriptionFinder.setWorkingModule(&rootModule);
    unitBuilder.setWorkingModule(&rootModule);
}

void ShnooTalkIRBuilder::setWorkingModule(icode::ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
    moduleBuilder.setWorkingModule(moduleDescription);
    descriptionFinder.setWorkingModule(moduleDescription);
    unitBuilder.setWorkingModule(moduleDescription);
}

void ShnooTalkIRBuilder::pushWorkingModule()
{
    moduleDescriptionStack.push_back(workingModule);
}

void ShnooTalkIRBuilder::popWorkingModule()
{
    setWorkingModule(moduleDescriptionStack.back());
    moduleDescriptionStack.pop_back();
}

void ShnooTalkIRBuilder::setWorkingFunction(const Token& functionNameToken)
{
    std::string mangledFunctionName = nameMangle(functionNameToken, rootModule.name);

    workingFunction = &rootModule.functions[mangledFunctionName];
    functionBuilder.setWorkingFunction(workingFunction);
    descriptionFinder.setWorkingFunction(workingFunction);
}
