#ifndef GENERATOR_EXPRESSION
#define GENERATOR_EXPRESSION

#include "GeneratorContext.hpp"

Unit term(generator::GeneratorContext& ctx, const Node& root);
Unit functionCall(generator::GeneratorContext& ctx, const Node& root);
Unit expression(generator::GeneratorContext& ctx, const Node& root);

#endif