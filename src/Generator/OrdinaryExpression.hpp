#ifndef GENERATOR_ORDINARY_EXPRESSION
#define GENERATOR_ORDINARY_EXPRESSION

#include "GeneratorContext.hpp"

Unit term(generator::GeneratorContext& ctx, const Node& root);
Unit functionCall(generator::GeneratorContext& ctx, const Node& root);
Unit ordinaryExpression(generator::GeneratorContext& ctx, const Node& root);

#endif