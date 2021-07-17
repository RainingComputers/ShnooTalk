#include "../Builder/NameMangle.hpp"

#include "GeneratorContext.hpp"

namespace generator
{
    GeneratorContext::GeneratorContext(icode::TargetEnums& target,
                                       icode::StringModulesMap& modulesMap,
                                       const std::string& fileName,
                                       Console& console)

      : rootModule(modulesMap[fileName])
      , console(console)
      , moduleBuilder(modulesMap, console)
      , unitBuilder(opBuilder)
      , descriptionFinder(modulesMap[fileName], modulesMap, unitBuilder, console)
      , functionBuilder(modulesMap, opBuilder, unitBuilder, console)
      , strBuilder(modulesMap[fileName], opBuilder)
    {
        workingFunction = nullptr;
        workingModule = &rootModule;

        rootModule.name = fileName;
        rootModule.initializeTarget(target);

        setWorkingModule(workingModule);
    }

    void GeneratorContext::resetWorkingModule()
    {
        workingModule = &rootModule;
        moduleBuilder.setWorkingModule(&rootModule);
        descriptionFinder.setWorkingModule(&rootModule);
    }

    void GeneratorContext::setWorkingModule(icode::ModuleDescription* moduleDescription)
    {
        workingModule = moduleDescription;
        moduleBuilder.setWorkingModule(moduleDescription);
        descriptionFinder.setWorkingModule(moduleDescription);
    }

    void GeneratorContext::pushWorkingModule()
    {
        moduleDescriptionStack.push_back(workingModule);
    }

    void GeneratorContext::popWorkingModule()
    {
        setWorkingModule(moduleDescriptionStack.back());
        moduleDescriptionStack.pop_back();
    }

    void GeneratorContext::setWorkingFunction(const Token& functionNameToken)
    {
        std::string mangledFunctionName = nameMangle(functionNameToken, rootModule.name);

        workingFunction = &rootModule.functions[mangledFunctionName];
        functionBuilder.setWorkingFunction(workingFunction);
        descriptionFinder.setWorkingFunction(workingFunction);
    }
} // namespace irgen
