#ifndef BUILDER_UNIT_BUILDER
#define BUILDER_UNIT_BUILDER

#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    OperandBuilder& opBuilder;

  public:
    UnitBuilder(OperandBuilder& opBuilder);

    Unit unitFromIntLiteral(int value, icode::DataType dtype);
    Unit unitFromFloatLiteral(float value, icode::DataType dtype);
    Unit unitFromTypeDescription(icode::TypeDescription& typeDescription, const std::string& name);
    Unit unitFromEnum(int enumValue);
    Unit unitFromDefineDescription(const icode::DefineDescription& defineDescription);
};

#endif