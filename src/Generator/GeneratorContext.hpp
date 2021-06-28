#ifndef GENERATOR_GENERATOR_CONTEXT
#define GENERATOR_GENERATOR_CONTEXT

#include "../Builder/DescriptionFinder.hpp"
#include "../Builder/FunctionBuilder.hpp"
#include "../Builder/ModuleBuilder.hpp"
#include "../Builder/StringBuilder.hpp"
#include "../Builder/UnitBuilder.hpp"
#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../IntermediateRepresentation/FunctionDescription.hpp"
#include "../Node/Node.hpp"
#include "ScopeTracker.hpp"

namespace generator
{
    class GeneratorContext
    {
      public:
        icode::TargetDescription& target;
        icode::StringModulesMap& modulesMap;
        icode::ModuleDescription& rootModule;
        Console& console;

        ModuleBuilder moduleBuilder;
        OperandBuilder opBuilder;
        UnitBuilder unitBuilder;
        DescriptionFinder descriptionFinder;
        FunctionBuilder functionBuilder;
        StringBuilder strBuilder;

        icode::FunctionDescription* workingFunction;
        icode::ModuleDescription* workingModule;

        ScopeTracker scope;

        std::vector<icode::ModuleDescription*> moduleDescriptionStack;

        void resetWorkingModule();
        void setWorkingModule(icode::ModuleDescription* moduleDescription);
        void pushWorkingModule();
        void popWorkingModule();

        void setWorkingFunction(icode::FunctionDescription* functionDescription);


        GeneratorContext(icode::TargetDescription& target_desc,
                     icode::StringModulesMap& modules_map,
                     const std::string& file_name,
                     Console& console);
    };
}

#endif