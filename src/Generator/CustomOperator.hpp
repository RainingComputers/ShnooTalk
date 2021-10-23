#ifndef GENERATOR_CUSTOM_OPERATOR
#define GENERATOR_CUSTOM_OPERATOR

#include "GeneratorContext.hpp"

Unit customOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS);

Unit binaryOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS);

#endif