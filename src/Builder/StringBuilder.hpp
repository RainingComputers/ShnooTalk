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

    icode::TypeDescription stringTypeFromToken(const Token& token);

    std::string createStringData(const Token& stringToken);

    icode::Operand createStringOperand(const Token& stringToken);

  public:
    StringBuilder(icode::ModuleDescription& rootModule, OperandBuilder& opBuilder);

    Unit createString(const Token& stringToken);
};

#endif