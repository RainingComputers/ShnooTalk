#ifndef GENERATOR_BINARY_OPERATOR
#define GENERATOR_BINARY_OPERATOR

#include "GeneratorContext.hpp"

Unit binaryOperator(generator::GeneratorContext& ctx, const Token& binaryOperator, const Unit& LHS, const Unit& RHS);

#endif