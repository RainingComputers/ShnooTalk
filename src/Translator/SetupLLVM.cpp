#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "SetupLLVM.hpp"

using namespace llvm;

/* Contains some LLVM boiler plate setup code */

void initializeTargetRegistry()
{
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
}

TargetMachine* setupTargetTripleAndDataLayout(const ModuleContext& ctx)
{
    auto targetTriple = sys::getDefaultTargetTriple();
    ctx.LLVMModule->setTargetTriple(targetTriple);

    std::string error;
    auto Target = TargetRegistry::lookupTarget(targetTriple, error);

    if (!Target)
        ctx.console.internalBugErrorMessage("LLVM ERROR: " + error);

    std::string CPU = "generic";
    std::string features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto targetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    ctx.LLVMModule->setDataLayout(targetMachine->createDataLayout());

    return targetMachine;
}

void setupPassManagerAndCreateObject(ModuleContext& ctx, TargetMachine* targetMachine)
{
    auto filename = ctx.moduleDescription.name + ".o";
    std::error_code EC;
    raw_fd_ostream dest(filename, EC, sys::fs::OF_None);

    if (EC)
        ctx.console.internalBugErrorMessage("LLVM ERROR: Could not open file: " + EC.message());

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        ctx.console.internalBugErrorMessage("LLVM ERROR: LLVM target machine can't emit a file of this type");

    pass.run(*ctx.LLVMModule);
    dest.flush();
}