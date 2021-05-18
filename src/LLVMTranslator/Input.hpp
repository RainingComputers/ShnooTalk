#ifndef INPUT_HPP
#define INPUT_HPP

#include "FormatStringsContext.hpp"
#include "ModuleContext.hpp"

void setupScanf(const ModuleContext& ctx);
void input(const ModuleContext& ctx, const FormatStringsContext& formatStringsContext, const icode::Entry& e);
void inputString(const ModuleContext& ctx, const icode::Entry& e);

#endif