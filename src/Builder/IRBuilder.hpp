#ifndef BUILDER_IRBUILDER
#define BUILDER_IRBUILDER

#include "DescriptionFinder.hpp"
#include "FunctionBuilder.hpp"
#include "ModuleBuilder.hpp"
#include "UnitBuilder.hpp"
#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/FunctionDescription.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Node/Node.hpp"

class IRBuilder
{
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

    void resetWorkingModule();
    void setWorkingModule(icode::ModuleDescription* moduleDescription);
    void pushWorkingModule();
    void popWorkingModule();

    void setWorkingFunction(const Token& functionNameToken);

    IRBuilder(icode::TargetEnums& target,
                     icode::StringModulesMap& modules_map,
                     const std::string& file_name,
                     Console& console);
};

#endif