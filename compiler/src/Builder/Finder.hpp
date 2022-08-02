#ifndef BUILDER_DESCRIPTION_FINDER
#define BUILDER_DESCRIPTION_FINDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"
#include "UnitBuilder.hpp"

class Finder
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
    Finder(icode::ModuleDescription& rootModule,
           icode::StringModulesMap& modulesMap,
           UnitBuilder& unitBuilder,
           Console& console);

    void setWorkingModule(icode::ModuleDescription* module);
    void setWorkingFunction(icode::FunctionDescription* function);

    icode::ModuleDescription* getModuleFromUnit(const Unit& unit);

    icode::ModuleDescription* getModuleFromType(const icode::TypeDescription& type);

    icode::ModuleDescription* getModuleFromToken(const Token& moduleNameToken);

    icode::StructDescription getStructDescFromType(const icode::TypeDescription& type);

    icode::StructDescription getStructDescFromUnit(const Unit& unit);

    std::vector<icode::TypeDescription> getFieldTypes(const icode::TypeDescription& type);

    std::vector<std::string> getFieldNames(const icode::TypeDescription& type);

    std::vector<icode::TypeDescription> destructureStructType(const icode::TypeDescription& type);

    std::map<std::string, icode::TypeDescription> destructureStructTypeMapped(const icode::TypeDescription& type);

    Unit getUnitFromToken(const Token& nameToken);

    icode::FunctionDescription getFunction(const Token& nameToken);

    std::vector<Unit> getFormalParameters(const icode::FunctionDescription& function);

    icode::FunctionDescription getFunctionByParamTypes(const Token& token,
                                                       const icode::TypeDescription& type,
                                                       const std::vector<Unit>& params);

    icode::FunctionDescription getCustomOperatorFunction(const std::string& binaryOperatorName,
                                                         const std::vector<Unit>& params,
                                                         const Token& errorToken);

    bool isAllNamesStructFields(const std::vector<Token>& nameTokens, const icode::TypeDescription& type);

    bool deconstructorExists(const icode::TypeDescription& type);

    bool methodExists(const icode::TypeDescription& type, const std::string& method);

    bool functionExists(const icode::TypeDescription& type, const std::string& method);

    icode::FunctionDescription getMethod(const icode::TypeDescription& type, const std::string& method);

    icode::FunctionDescription getMethodFromUnit(const Unit& unit, const std::string& method, const Token& errorToken);
};

#endif