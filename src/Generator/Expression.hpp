#ifndef GENERATOR_EXPRESSION
#define GENERATOR_EXPRESSION

#include "GeneratorContext.hpp"

Unit functionCall(generator::GeneratorContext& ctx, const Node& root);

Unit term(generator::GeneratorContext& ctx, const Node& root);

Unit ordinaryExpression(generator::GeneratorContext& ctx, const Node& root);

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall);

Unit expression(generator::GeneratorContext& ctx, const Node& root);

#endif