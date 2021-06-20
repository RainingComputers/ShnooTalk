#ifndef BUILDER_STRING_BUILDER
#define BUILDER_STRING_BUILDER

#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"

class StringBuilder
{
    icode::ModuleDescription& rootModule;
    OperandBuilder& opBuilder;

  public:
    StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder);

    icode::Operand createStringOperand(const Token& stringToken);

    Unit createString(const Token& stringToken);
};

#endif