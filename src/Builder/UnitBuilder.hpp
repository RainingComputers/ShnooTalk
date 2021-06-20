#ifndef UNIT_BUILDER
#define UNIT_BUILDER

#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    OperandBuilder& opBuilder;

  public:
    UnitBuilder(OperandBuilder& opBuilder);

    Unit unitFromIntLiteral(int value, icode::DataType dtype);
    Unit unitFromFloatLiteral(float value, icode::DataType dtype);
    Unit unitPairFromTypeDescription(icode::TypeDescription& typeDescription, const std::string& name);
    Unit unitFromEnum(int enumValue);
    Unit unitFromDefineDescription(const icode::DefineDescription& defineDescription);
};

#endif