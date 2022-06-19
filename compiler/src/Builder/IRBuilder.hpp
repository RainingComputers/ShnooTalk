#ifndef BUILDER_IRBUILDER
#define BUILDER_IRBUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/FunctionDescription.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Node/Node.hpp"
#include "Finder.hpp"
#include "FunctionBuilder.hpp"
#include "ModuleBuilder.hpp"
#include "UnitBuilder.hpp"

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
    Finder finder;
    FunctionBuilder functionBuilder;

    void resetWorkingModule();
    void setWorkingModule(icode::ModuleDescription* moduleDescription);
    void pushWorkingModule();
    void popWorkingModule();

    void setWorkingFunction(const Token& functionNameToken, bool externC);

    IRBuilder(icode::StringModulesMap& modules_map, const std::string& file_name, Console& console);
};

#endif