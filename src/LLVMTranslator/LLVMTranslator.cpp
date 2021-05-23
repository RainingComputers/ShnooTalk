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

    target.defines = { { "true", icode::createIntDefine(1, icode::INT) },
                       { "false", icode::createIntDefine(0, icode::INT) } };

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

std::string llvmgen::generateLLVMModule(icode::ModuleDescription& modDesc,
                                        icode::StringModulesMap& modulesMap,
                                        bool createObject)
{
    ModuleContext moduleContext(modDesc, modulesMap);
    BranchContext branchContext;
    FormatStringsContext formatStringsContext;

    generateModule(moduleContext, branchContext, formatStringsContext);

    if (createObject)
    {
        initializeTargetRegistry();
        TargetMachine* targetMachine = setupTargetTripleAndDataLayout(moduleContext);
        setupPassManagerAndCreateObject(moduleContext, targetMachine);
    }

    return getLLVMModuleString(*moduleContext.LLVMModule);
}
