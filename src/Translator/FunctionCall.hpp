#ifndef TRANSLATOR_FUNCTION_CALL
#define TRANSLATOR_FUNCTION_CALL

#include "ModuleContext.hpp"

void call(ModuleContext& ctx, const icode::Entry& e);
void ret(const ModuleContext& ctx, const icode::Entry& e, icode::DataType dtype);
void pass(ModuleContext& ctx, const icode::Entry& e);
void passPointer(ModuleContext& ctx, const icode::Entry& e);

#endif