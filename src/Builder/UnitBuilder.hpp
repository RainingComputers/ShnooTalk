#ifndef BUILDER_UNIT_BUILDER
#define BUILDER_UNIT_BUILDER

#include "ModuleBuilder.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    icode::ModuleDescription& rootModule;
    OperandBuilder& opBuilder;

    icode::ModuleDescription* workingModule;

    int getCharCountFromStringDataKey(const std::string& key);

public:
    UnitBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder);

    void setWorkingModule(icode::ModuleDescription* moduleDescription);

    Unit unitFromIntLiteral(int value);
    Unit unitFromCharLiteral(char value);
    Unit unitFromFloatLiteral(float value);
    Unit unitFromTypeDescription(icode::TypeDescription& typeDescription, const std::string& name);
    Unit unitFromEnum(int enumValue);
    Unit unitFromUnitList(const std::vector<Unit>& unitList);
    Unit unitFromStringDataKey(const std::string& key);
};

#endif