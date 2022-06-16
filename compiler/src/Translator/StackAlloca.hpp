#ifndef TRANSLATOR_STACK_ALLOCA
#define TRANSLATOR_STACK_ALLOCA

#include <string>

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

llvm::Value* stackAllocName(const ModuleContext& ctx, llvm::Type* type, const std::string& name);
llvm::Value* stackAlloc(const ModuleContext& ctx, llvm::Type* type);

#endif