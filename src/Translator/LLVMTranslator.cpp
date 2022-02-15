#include <string>

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Passes/PassBuilder.h"

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

void optimizeModule(ModuleContext& ctx)
{
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    PassBuilder PB;

    FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(PassBuilder::OptimizationLevel::O3);

    MPM.run(*ctx.LLVMModule, MAM);
}

void translator::generateLLVMModuleObject(icode::ModuleDescription& moduleDescription,
                                          icode::StringModulesMap& modulesMap,
                                          bool release,
                                          Console& console)
{
    ModuleContext moduleContext(moduleDescription, modulesMap, console);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    generateModule(moduleContext, branchContext, formatStringsContext);

    initializeTargetRegistry();
    TargetMachine* targetMachine = setupTargetTripleAndDataLayout(moduleContext);
    if (release)
        optimizeModule(moduleContext);
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