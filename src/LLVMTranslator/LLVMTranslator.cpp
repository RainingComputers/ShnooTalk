#include <string>

#include "BranchContext.hpp"
#include "FormatStringsContext.hpp"
#include "GenerateModule.hpp"
#include "ModuleContext.hpp"
#include "Print.hpp"
#include "SetupLLVM.hpp"

#include "LLVMTranslator.hpp"

using namespace llvm;

icode::TargetDescription llvmgen::getTargetDescription()
{
    icode::TargetDescription target;

    target.dataTypeNames = { { "byte", icode::I8 },     { "ubyte", icode::UI8 },  { "short", icode::I16 },
                             { "ushort", icode::UI16 }, { "int", icode::I32 },    { "uint", icode::UI32 },
                             { "long", icode::I64 },    { "ulong", icode::UI64 }, { "float", icode::F32 },
                             { "double", icode::F64 },  { "char", icode::UI8 },   { "bool", icode::UI8 } };

    target.defines = { { "true", icode::createIntDefineDescription(1, icode::INT) },
                       { "false", icode::createIntDefineDescription(0, icode::INT) } };

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

void llvmgen::generateLLVMModuleObject(icode::ModuleDescription& moduleDescription,
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

std::string llvmgen::generateLLVMModuleString(icode::ModuleDescription& moduleDescription,
                                              icode::StringModulesMap& modulesMap,
                                              Console& console)
{
    ModuleContext moduleContext(moduleDescription, modulesMap, console);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    generateModule(moduleContext, branchContext, formatStringsContext);

    return getLLVMModuleString(*moduleContext.LLVMModule);
}