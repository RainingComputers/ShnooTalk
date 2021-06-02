#ifndef UNIT_BUILDER
#define UNIT_BUILDER

#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"

class UnitBuilder
{
    OperandBuilder& opBuilder;

  public:
    UnitBuilder(OperandBuilder& opBuilder);

    Unit unitPairFromIntLiteral(int value, icode::DataType dtype);
    Unit unitPairFromFloatLiteral(float value, icode::DataType dtype);
    Unit unitPairFromTypeDescription(icode::TypeDescription& typeDescription,
                                                       const token::Token& nameToken);
    Unit unitFromEnum(int enumValue);
    Unit unitFromDefineDescription(const icode::DefineDescription& defineDescription);
};

#endif