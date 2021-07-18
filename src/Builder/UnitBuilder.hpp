#ifndef BUILDER_UNIT_BUILDER
#define BUILDER_UNIT_BUILDER

#include "ModuleBuilder.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    icode::ModuleDescription& rootModule;
    OperandBuilder& opBuilder;

  public:
    UnitBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder);

    Unit unitFromIntLiteral(int value);
    Unit unitFromFloatLiteral(float value);
    Unit unitFromTypeDescription(icode::TypeDescription& typeDescription, const std::string& name);
    Unit unitFromEnum(int enumValue);
    Unit unitFromUnitList(const std::vector<Unit>& unitList);
    Unit unitFromStringDataKey(const std::string& key);
};

#endif