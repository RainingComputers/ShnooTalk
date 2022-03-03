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
    UnitBuilder& unitBuilder;
    Console& console;

    icode::ModuleDescription* workingModule;
    icode::FunctionDescription* workingFunction;

    bool getLocal(const Token& nameToken, Unit& returnValue);
    bool getGlobal(const Token& nameToken, Unit& returnValue);
    bool getIntDefine(const Token& nameToken, Unit& returnValue);
    bool getFloatDefine(const Token& nameToken, Unit& returnValue);
    bool getStringDefine(const Token& nameToken, Unit& returnValue);
    bool getEnum(const Token& nameToken, Unit& returnValue);

public:
    DescriptionFinder(icode::ModuleDescription& rootModule,
                      icode::StringModulesMap& modulesMap,
                      UnitBuilder& unitBuilder,
                      Console& console);

    void setWorkingModule(icode::ModuleDescription* module);
    void setWorkingFunction(icode::FunctionDescription* function);

    icode::ModuleDescription* getModuleFromUnit(const Unit& unit);

    icode::ModuleDescription* getModuleFromType(const icode::TypeDescription& type);

    icode::ModuleDescription* getModuleFromToken(const Token& moduleNameToken);

    Unit getUnitFromToken(const Token& nameToken);

    unsigned int getDataTypeSizeFromToken(const Token& nameToken);

    icode::FunctionDescription getFunction(const Token& nameToken);

    std::vector<Unit> getFormalParameters(const icode::FunctionDescription& function);

    std::pair<std::string, icode::FunctionDescription> getFunctionByParamTypes(const Token& token,
                                              const icode::TypeDescription& type,
                                              const std::vector<Unit>& params);
};

#endif