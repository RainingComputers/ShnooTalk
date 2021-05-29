#ifndef LLVM_TRANSLATOR
#define LLVM_TRANSLATOR

#include "../IntermediateRepresentation/All.hpp"

namespace llvmgen
{
    icode::TargetDescription getTargetDescription();
    void generateLLVMModuleObject(icode::ModuleDescription& moduleDescription,
                                  icode::StringModulesMap& modulesMap,
                                  Console& console);
    std::string generateLLVMModuleString(icode::ModuleDescription& moduleDescription,
                                         icode::StringModulesMap& modulesMap,
                                         Console& console);
}

#endif