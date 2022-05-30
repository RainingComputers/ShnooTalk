#include "NameMangle.hpp"

#include "IRBuilder.hpp"

IRBuilder::IRBuilder(icode::StringModulesMap& modulesMap, const std::string& fileName, Console& console)

    : rootModule(modulesMap[fileName])
    , console(console)
    , moduleBuilder(modulesMap[fileName], modulesMap, console)
    , unitBuilder(modulesMap[fileName], opBuilder)
    , finder(modulesMap[fileName], modulesMap, unitBuilder, console)
    , functionBuilder(modulesMap, opBuilder, unitBuilder, finder, console)
{
    workingFunction = nullptr;
    workingModule = &rootModule;

    rootModule.name = fileName;

    setWorkingModule(workingModule);
}

void IRBuilder::resetWorkingModule()
{
    workingModule = &rootModule;
    moduleBuilder.setWorkingModule(&rootModule);
    finder.setWorkingModule(&rootModule);
    unitBuilder.setWorkingModule(&rootModule);
}

void IRBuilder::setWorkingModule(icode::ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
    moduleBuilder.setWorkingModule(moduleDescription);
    finder.setWorkingModule(moduleDescription);
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

void IRBuilder::setWorkingFunction(const Token& functionNameToken, bool externC)
{
    if (externC)
        workingFunction = &rootModule.functions.at(functionNameToken.toString());
    else
    {
        std::string mangledFunctionName = nameMangle(functionNameToken, rootModule.name);
        workingFunction = &rootModule.functions.at(mangledFunctionName);
    }

    functionBuilder.setWorkingFunction(workingFunction);
    finder.setWorkingFunction(workingFunction);
}
