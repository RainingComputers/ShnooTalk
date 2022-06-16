#ifndef TRANSLATOR_LLVM_SETUP
#define TRANSLATOR_LLVM_SETUP

#include <string>

#include "llvm/Target/TargetMachine.h"

#include "ModuleContext.hpp"

void initializeTargetRegistry();
void setupPassManagerAndCreateObject(ModuleContext& ctx, const std::string& targetTriple);

#endif