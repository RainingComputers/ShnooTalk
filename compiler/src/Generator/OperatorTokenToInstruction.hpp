#ifndef GENERATOR_OPERATOR_TOKEN_TO_INSTRUCTION
#define GENERATOR_OPERATOR_TOKEN_TO_INSTRUCTION

#include "GeneratorContext.hpp"

icode::Instruction tokenToBinaryOperator(const generator::GeneratorContext& ctx, const Token& tok);
icode::Instruction tokenToCompareOperator(const generator::GeneratorContext& ctx, const Token& tok);
std::string tokenToCustomOperatorString(const generator::GeneratorContext& ctx, const Token& tok);

#endif