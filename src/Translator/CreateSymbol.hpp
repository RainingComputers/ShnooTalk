#ifndef TRANSLATOR_CREATE_SYMBOL
#define TRANSLATOR_CREATE_SYMBOL

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

void createGlobalString(ModuleContext& ctx, const std::string& key, const std::string& str);

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             const std::string& name,
                             llvm::Value* arg);

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

#endif