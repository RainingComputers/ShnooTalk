#ifndef TRANSLATOR_LLVM_SETUP
#define TRANSLATOR_LLVM_SETUP

#include "llvm/Target/TargetMachine.h"

#include "ModuleContext.hpp"

void initializeTargetRegistry();
llvm::TargetMachine* setupTargetTripleAndDataLayout(const ModuleContext& ctx);
void setupPassManagerAndCreateObject(ModuleContext& ctx, llvm::TargetMachine* targetMachine);

#endif