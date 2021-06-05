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

    icode::ModuleDescription* getModuleFromToken(const Token& moduleNameToken,
                                                 icode::StringModulesMap& modulesMap);

    bool getLocal(const Token& nameToken, Unit& returnValue);
    bool getGlobal(const Token& nameToken, Unit& returnValue);
    bool getDefine(const Token& nameToken, Unit& returnValue);
    bool getEnum(const Token& nameToken, Unit& returnValue);

    Unit getUnitFromToken(const Token& nameToken);
};

#endif