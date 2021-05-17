#ifndef LLVM_TRANSLATOR
#define LLVM_TRANSLATOR

#include "../IntermediateRepresentation/All.hpp"

namespace llvmgen
{
    icode::TargetDescription getTargetDescription();
    std::string generateLLVMModule(icode::ModuleDescription& modDesc,
                                   icode::StringModulesMap& modulesMap,
                                   bool createObject);
}

#endif