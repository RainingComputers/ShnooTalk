#ifndef BUILDER_UNIT_BUILDER
#define BUILDER_UNIT_BUILDER

#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    OperandBuilder& opBuilder;

  public:
    UnitBuilder(OperandBuilder& opBuilder);

    Unit unitFromIntLiteral(int value);
    Unit unitFromFloatLiteral(float value);
    Unit unitFromTypeDescription(icode::TypeDescription& typeDescription, const std::string& name);
    Unit unitFromEnum(int enumValue);
    Unit unitFromDefineDescription(const icode::DefineDescription& defineDescription);
    Unit unitFromUnitList(const std::vector<Unit>& unitList);
};

#endif