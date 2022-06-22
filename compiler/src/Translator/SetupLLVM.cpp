#include <filesystem>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "../Builder/NameMangle.hpp"

#include "SetupLLVM.hpp"

using namespace llvm;

/* Contains some LLVM setup code */

std::string getTargetTriple(translator::Platform platform)
{
    std::map<translator::Platform, std::string> platformTripleMap = {
        { translator::DEFAULT, sys::getDefaultTargetTriple() },
        { translator::LINUX_x86_64, "x86_64-linux-gnu" },
        { translator::LINUX_ARM64, "arm64-linux-gnu" },
        { translator::MACOS_x86_64, "x86_64-apple-darwin" },
        { translator::MACOS_ARM64, "arm64-apple-darwin" },
        { translator::WASM32, "wasm32" },
        { translator::WASM64, "wasm64" },
        { translator::LINUX_x86_64_DYN, "x86_64-linux-gnu" },
        { translator::LINUX_ARM64_DYN, "arm64-linux-gnu" },
        { translator::MACOS_x86_64_DYN, "x86_64-apple-darwin" },
        { translator::MACOS_ARM64_DYN, "arm64-apple-darwin" },
    };

    return platformTripleMap.at(platform);
}

Reloc::Model getRelocModel(translator::Platform platform)
{
    std::map<translator::Platform, Reloc::Model> platformRelocMap = {
        { translator::DEFAULT, Reloc::Model::PIC_ },
        { translator::LINUX_x86_64, Reloc::Model::PIC_ },
        { translator::LINUX_ARM64, Reloc::Model::PIC_ },
        { translator::MACOS_x86_64, Reloc::Model::PIC_ },
        { translator::MACOS_ARM64, Reloc::Model::PIC_ },
        { translator::WASM32, Reloc::Model::PIC_ },
        { translator::WASM64, Reloc::Model::PIC_ },
        { translator::LINUX_x86_64_DYN, Reloc::Model::DynamicNoPIC },
        { translator::LINUX_ARM64_DYN, Reloc::Model::DynamicNoPIC },
        { translator::MACOS_x86_64_DYN, Reloc::Model::DynamicNoPIC },
        { translator::MACOS_ARM64_DYN, Reloc::Model::DynamicNoPIC },
    };

    return platformRelocMap.at(platform);
}

void initializeTargetRegistry()
{
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
}

TargetMachine* setupTargetTripleAndDataLayout(const ModuleContext& ctx,
                                              const std::string& targetTriple,
                                              llvm::Reloc::Model relocModel)
{
    std::string error;
    auto Target = TargetRegistry::lookupTarget(targetTriple, error);

    if (!Target)
        ctx.console.internalBugErrorMessage("LLVM ERROR: " + error);

    std::string CPU = "generic";
    std::string features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>(relocModel);
    auto targetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    ctx.LLVMModule->setDataLayout(targetMachine->createDataLayout());

    return targetMachine;
}

std::string createDirsAndGetOutputObjNameStatic(const std::string& moduleName)
{
    std::filesystem::path objFileName(mangleModuleName(moduleName));
    objFileName += ".o";

    const std::filesystem::path objDir("_obj");
    const std::filesystem::path objPath = objDir / objFileName;

    std::filesystem::create_directory(objDir);

    return objPath.string();
}

std::string createDirsAndGetOutputObjName(const std::string& moduleName, translator::Platform platform)
{
    if (platform == translator::MACOS_ARM64_DYN || platform == translator::MACOS_x86_64_DYN)
        return moduleName + ".dylib";

    if (platform == translator::LINUX_ARM64_DYN || platform == translator::LINUX_x86_64_DYN)
        return moduleName + ".so";

    return createDirsAndGetOutputObjNameStatic(moduleName);
}

void setupPassManagerAndCreateObject(ModuleContext& ctx, translator::Platform platform)
{
    Reloc::Model relocModel = getRelocModel(platform);
    std::string targetTriple = getTargetTriple(platform);

    TargetMachine* targetMachine = setupTargetTripleAndDataLayout(ctx, targetTriple, relocModel);

    std::string filename = createDirsAndGetOutputObjName(ctx.moduleDescription.name, platform);

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