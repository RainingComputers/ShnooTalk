#ifndef PRINT_HPP
#define PRINT_HPP

#include "FormatStringsContext.hpp"
#include "ModuleContext.hpp"

void setupPrintf(const ModuleContext& ctx);
void callPrintf(const ModuleContext& ctx, llvm::Value* formatString, llvm::Value* value);
void print(const ModuleContext& ctx, const FormatStringsContext& formatStringsContext, const icode::Entry& e);
void printString(const ModuleContext& ctx, const icode::Entry& e);

#endif