#ifndef CREATE_SYMBOL_HPP
#define CREATE_SYMBOL_HPP

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             const std::string& name,
                             llvm::Value* arg);

#endif