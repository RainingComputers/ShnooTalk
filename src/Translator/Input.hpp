#ifndef TRANSLATOR_INPUT
#define TRANSLATOR_INPUT

#include "FormatStringsContext.hpp"
#include "ModuleContext.hpp"

void setupScanf(const ModuleContext& ctx);
void input(const ModuleContext& ctx, const FormatStringsContext& formatStringsContext, const icode::Entry& e);
void inputString(const ModuleContext& ctx, const icode::Entry& e);

#endif