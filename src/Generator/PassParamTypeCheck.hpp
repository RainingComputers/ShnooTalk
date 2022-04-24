#ifndef GENERATOR_PASS_PARAM_TYPE_CHECK
#define GENERATOR_PASS_PARAM_TYPE_CHECK

#include "GeneratorContext.hpp"

void passParamCheck(const generator::GeneratorContext& ctx,
                        const Unit& actualParam,
                        const Unit& formalParam,
                        const Token& actualParamToken);

#endif