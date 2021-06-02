#ifndef VALUE_BUILDER
#define VALUE_BUILDER

#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "OperandDescriptionPair.hpp"

class ValueBuilder
{
    OperandBuilder& opBuilder;

  public:
    ValueBuilder(OperandBuilder& opBuilder);

    OperandDescriptionPair operandDescPairFromIntLiteral(int value, icode::DataType dtype);
    OperandDescriptionPair operandDescPairFromFloatLiteral(float value, icode::DataType dtype);
    OperandDescriptionPair operandDescPairFromTypeDesc(icode::TypeDescription& typeDescription,
                                                       const token::Token& nameToken);
    OperandDescriptionPair operandDescPairFromEnum(int enumValue);
    OperandDescriptionPair operandDescPairFromDefine(const icode::DefineDescription& defineDescription);
};

#endif