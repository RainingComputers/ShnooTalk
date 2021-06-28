#ifndef TRANSLATOR_LLVM_TRANSLATOR
#define TRANSLATOR_LLVM_TRANSLATOR

#include "../IntermediateRepresentation/All.hpp"

namespace translator
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