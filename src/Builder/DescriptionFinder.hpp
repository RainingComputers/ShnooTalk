#ifndef BUILDER_DESCRIPTION_FINDER
#define BUILDER_DESCRIPTION_FINDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"
#include "UnitBuilder.hpp"

class DescriptionFinder
{
    icode::ModuleDescription& rootModule;
    icode::StringModulesMap& modulesMap;
    icode::ModuleDescription* workingModule;
    icode::FunctionDescription* workingFunction;

    Console& console;

    UnitBuilder& unitBuilder;

    bool getLocal(const Token& nameToken, Unit& returnValue);
    bool getGlobal(const Token& nameToken, Unit& returnValue);
    bool getDefine(const Token& nameToken, Unit& returnValue);
    bool getEnum(const Token& nameToken, Unit& returnValue);

  public:
    DescriptionFinder(icode::ModuleDescription& rootModule,
                      icode::StringModulesMap& modulesMap,
                      Console& console,
                      UnitBuilder& unitBuilder);

    void setWorkingModule(icode::ModuleDescription* module);
    void setWorkingFunction(icode::FunctionDescription* function);

    icode::ModuleDescription* getModuleFromUnit(const Unit& unit);

    icode::ModuleDescription* getModuleFromToken(const Token& moduleNameToken);

    Unit getUnitFromToken(const Token& nameToken);

    int getDataTypeSizeFromToken(const Token& nameToken);

    icode::FunctionDescription getFunction(const Token& nameToken);

    std::vector<Unit> getFormalParameters(const icode::FunctionDescription& function);
};

#endif