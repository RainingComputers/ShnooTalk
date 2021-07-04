#ifndef GENERATOR_GENERATOR_CONTEXT
#define GENERATOR_GENERATOR_CONTEXT

#include "../Builder/DescriptionFinder.hpp"
#include "../Builder/FunctionBuilder.hpp"
#include "../Builder/ModuleBuilder.hpp"
#include "../Builder/StringBuilder.hpp"
#include "../Builder/TypeChecker.hpp"
#include "../Builder/UnitBuilder.hpp"
#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/FunctionDescription.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Node/Node.hpp"
#include "ScopeTracker.hpp"

namespace generator
{
    class GeneratorContext
    {
        icode::TargetEnums& target;
        icode::StringModulesMap& modulesMap;
        icode::ModuleDescription& rootModule;

        icode::FunctionDescription* workingFunction;
        icode::ModuleDescription* workingModule;

        std::vector<icode::ModuleDescription*> moduleDescriptionStack;

      public:
        Console& console;

        ModuleBuilder moduleBuilder;
        OperandBuilder opBuilder;
        UnitBuilder unitBuilder;
        DescriptionFinder descriptionFinder;
        FunctionBuilder functionBuilder;
        StringBuilder strBuilder;
        TypeChecker typeChecker;

        ScopeTracker scope;

        void resetWorkingModule();
        void setWorkingModule(icode::ModuleDescription* moduleDescription);
        void pushWorkingModule();
        void popWorkingModule();

        void setWorkingFunction(const Token& functionNameToken);

        GeneratorContext(icode::TargetEnums& target,
                         icode::StringModulesMap& modules_map,
                         const std::string& file_name,
                         Console& console);
    };
}

#endif