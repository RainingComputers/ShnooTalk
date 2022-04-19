#ifndef TRANSLATOR_CREATE_SYMBOL
#define TRANSLATOR_CREATE_SYMBOL

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

void createGlobalString(ModuleContext& ctx, const std::string& key, const std::string& str);

llvm::Value* createExternGlobalString(ModuleContext& ctx, const std::string& key);

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             llvm::Value* arg,
                             const std::string& name);

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name);

#endif