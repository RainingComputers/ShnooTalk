#ifndef TRANSLATOR_LLVM_SETUP
#define TRANSLATOR_LLVM_SETUP

#include "llvm/Target/TargetMachine.h"

#include "ModuleContext.hpp"

void initializeTargetRegistry();
void setupPassManagerAndCreateObject(ModuleContext& ctx);

#endif