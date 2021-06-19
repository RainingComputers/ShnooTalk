#ifndef STRING_BUILDER_HPP
#define STRING_BUILDER_HPP

#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"
#include "../Token/Token.hpp"

class StringBuilder
{
    icode::ModuleDescription& rootModule;
    OperandBuilder& opBuilder;

  public:
    StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder);

    icode::Operand creatStringOperand(const Token& str_token, icode::DataType dtype);

    Unit createString(const Token& str_token);
};

#endif