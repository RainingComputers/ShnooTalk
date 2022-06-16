#ifndef TRANSLATOR_LLVM_TRANSLATOR
#define TRANSLATOR_LLVM_TRANSLATOR

#include "../IntermediateRepresentation/All.hpp"

#include "ModuleContext.hpp"

namespace translator
{
    enum Platform {
        DEFAULT,
        LINUX_x86_64,
        LINUX_ARM64,
        MACOS_x86_64,
        MACOS_ARM64,
        WASM32,
        WASM64,
    };

    void generateObject(icode::ModuleDescription& moduleDescription,
                        icode::StringModulesMap& modulesMap,
                        Platform platform,
                        bool release,
                        Console& console);

    std::string generateLLVMModuleString(icode::ModuleDescription& moduleDescription,
                                         icode::StringModulesMap& modulesMap,
                                         bool release,
                                         Console& console);
}

#endif