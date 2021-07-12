#include <string>

#include "BranchContext.hpp"
#include "FormatStringsContext.hpp"
#include "GenerateModule.hpp"
#include "ModuleContext.hpp"
#include "Print.hpp"
#include "SetupLLVM.hpp"

#include "LLVMTranslator.hpp"

using namespace llvm;

icode::TargetEnums translator::getTarget()
{
    icode::TargetEnums target;

    target.defines = { { "true", icode::createIntDefineDescription(1, icode::AUTO_INT) },
                       { "false", icode::createIntDefineDescription(0, icode::AUTO_INT) } };

    return target;
}

std::string getLLVMModuleString(const Module& LLVMModule)
{
    std::string moduleString;
    raw_string_ostream OS(moduleString);
    OS << LLVMModule;
    OS.flush();

    return moduleString;
}

void translator::generateLLVMModuleObject(icode::ModuleDescription& moduleDescription,
                                          icode::StringModulesMap& modulesMap,
                                          Console& console)
{
    ModuleContext moduleContext(moduleDescription, modulesMap, console);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    generateModule(moduleContext, branchContext, formatStringsContext);

    initializeTargetRegistry();
    TargetMachine* targetMachine = setupTargetTripleAndDataLayout(moduleContext);
    setupPassManagerAndCreateObject(moduleContext, targetMachine);
}

std::string translator::generateLLVMModuleString(icode::ModuleDescription& moduleDescription,
                                                 icode::StringModulesMap& modulesMap,
                                                 Console& console)
{
    ModuleContext moduleContext(moduleDescription, modulesMap, console);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    generateModule(moduleContext, branchContext, formatStringsContext);

    return getLLVMModuleString(*moduleContext.LLVMModule);
}