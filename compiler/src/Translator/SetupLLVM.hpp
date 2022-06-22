#ifndef TRANSLATOR_LLVM_SETUP
#define TRANSLATOR_LLVM_SETUP

#include <string>

#include "llvm/Target/TargetMachine.h"

#include "ModuleContext.hpp"
#include "Platform.hpp"

void initializeTargetRegistry();
void setupPassManagerAndCreateObject(ModuleContext& ctx,
                                     translator::Platform platform);

#endif