#ifndef GENERATOR_PASS_PARAM_TYPE_CHECK
#define GENERATOR_PASS_PARAM_TYPE_CHECK

#include "GeneratorContext.hpp"

void passParamCheck(const generator::GeneratorContext& ctx,
                    const Unit& formalParam,
                    const Unit& actualParam,
                    const Token& actualParamToken);

#endif