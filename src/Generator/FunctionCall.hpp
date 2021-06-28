#ifndef GENERATOR_FUNCTION_CALL
#define GENERATOR_FUNCTION_CALL

#include "GeneratorContext.hpp"

Unit functionCall(generator::GeneratorContext& ctx, const Node& root);
void functionReturn(generator::GeneratorContext& ctx, const Node& node);

#endif