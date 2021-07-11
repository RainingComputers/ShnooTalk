#ifndef GENERATOR_CONDITIONAL_EXPRESSION
#define GENERATOR_CONDITIONAL_EXPRESSION

#include "GeneratorContext.hpp"

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall);

#endif
