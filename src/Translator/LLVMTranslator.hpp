#ifndef TRANSLATOR_LLVM_TRANSLATOR
#define TRANSLATOR_LLVM_TRANSLATOR

#include "../IntermediateRepresentation/All.hpp"

#include "ModuleContext.hpp"

namespace translator
{
    void generateLLVMModule(ModuleContext& ctx, bool release, Console& console);

    void generateObject(icode::ModuleDescription& moduleDescription,
                        icode::StringModulesMap& modulesMap,
                        bool release,
                        Console& console);

    std::string generateLLVMModuleString(icode::ModuleDescription& moduleDescription,
                                         icode::StringModulesMap& modulesMap,
                                         bool release,
                                         Console& console);
}

#endif