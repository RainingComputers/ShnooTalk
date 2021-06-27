#ifndef TRANSLATOR_READ_WRITE_COPY
#define TRANSLATOR_READ_WRITE_COPY

#include "ModuleContext.hpp"

void createPointer(ModuleContext& ctx, const icode::Entry& e);
void copy(ModuleContext& ctx, const icode::Entry& e);
void read(ModuleContext& ctx, const icode::Entry& e);
void write(const ModuleContext& ctx, const icode::Entry& e);
void addressBinaryOperator(ModuleContext& ctx, const icode::Entry& e);
void memCopy(ModuleContext& ctx, const icode::Entry& e);

#endif