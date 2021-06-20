#ifndef STRING_BUILDER_HPP
#define STRING_BUILDER_HPP

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

    icode::Operand createStringOperand(const Token& str_token);

    Unit createString(const Token& str_token);
};

#endif