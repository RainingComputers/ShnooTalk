#ifndef TRANSLATOR_UNARY_OPERATOR
#define TRANSLATOR_UNARY_OPERATOR

#include "ModuleContext.hpp"

void bitwiseNot(ModuleContext& ctx, const icode::Entry& e);
void unaryMinus(ModuleContext& ctx, const icode::Entry& e);

#endif