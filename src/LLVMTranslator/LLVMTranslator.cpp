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
    /* Target description for mikuro-llvm */
    icode::TargetDescription target;

    target.dataTypeNames = { { "byte", icode::I8 },     { "ubyte", icode::UI8 },  { "short", icode::I16 },
                             { "ushort", icode::UI16 }, { "int", icode::I32 },    { "uint", icode::UI32 },
                             { "long", icode::I64 },    { "ulong", icode::UI64 }, { "float", icode::F32 },
                             { "double", icode::F64 },  { "char", icode::UI8 },   { "bool", icode::UI8 } };

    /* true and false defines */
    icode::Define trueDef;
    trueDef.dtype = icode::INT;
    trueDef.val.integer = 1;

    icode::Define falseDef;
    falseDef.dtype = icode::INT;
    falseDef.val.integer = 0;

    target.defines = { { "true", trueDef }, { "false", falseDef } };

    /* Default int or word */
    target.characterInt = icode::UI8;

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

std::string llvmgen::generateObjectFile(icode::ModuleDescription& modDesc, icode::StringModulesMap& modulesMap)
{
    ModuleContext moduleContext(modDesc, modulesMap);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    setupPrintf(moduleContext, formatStringsContext);
    generateModule(moduleContext, branchContext, formatStringsContext);
    initializeTargetRegistry();
    TargetMachine* targetMachine = setupTargetTripleAndDataLayout(moduleContext);
    setupPassManagerAndCreateObject(moduleContext, targetMachine);

    return getLLVMModuleString(*moduleContext.LLVMModule);
}
