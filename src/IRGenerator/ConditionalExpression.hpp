#ifndef GENERATOR_CONDITIONAL_EXPRESSION
#define GENERATOR_CONDITIONAL_EXPRESSION

#include "../irgen_old.hpp"

void conditionalExpression(irgen::ir_generator& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall);

#endif
