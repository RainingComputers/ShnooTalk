#ifndef DESCRIPTION_FINDER
#define DESCRIPTION_FINDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"
#include "UnitBuilder.hpp"

class DescriptionFinder
{
    icode::ModuleDescription& rootModule;
    icode::ModuleDescription* workingModule;
    icode::FunctionDescription* workingFunction;

    Console& console;

    UnitBuilder& unitBuilder;

  public:
    DescriptionFinder(icode::ModuleDescription& rootModule, Console& console, UnitBuilder& unitBuilder);

    void setWorkingModule(icode::ModuleDescription* module);
    void setWorkingFunction(icode::FunctionDescription* function);

    icode::ModuleDescription* getModuleFromToken(const token::Token& moduleNameToken,
                                                 icode::StringModulesMap& modulesMap);

    bool getLocal(const token::Token& nameToken, Unit& returnValue);
    bool getGlobal(const token::Token& nameToken, Unit& returnValue);
    bool getDefine(const token::Token& nameToken, Unit& returnValue);
    bool getEnum(const token::Token& nameToken, Unit& returnValue);

    Unit getUnitFromToken(const token::Token& nameToken);
};

#endif