#ifndef TRANSLATOR_MODULE_CONTEXT
#define TRANSLATOR_MODULE_CONTEXT

#include <map>
#include <queue>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"

struct ModuleContext
{
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> LLVMModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    std::map<std::string, llvm::Value*> symbolNamePointersMap;
    std::map<std::string, llvm::GlobalVariable*> symbolNameGlobalsMap;
    std::map<std::string, llvm::Value*> operandGlobalStringMap;
    std::map<icode::Operand, llvm::Value*> operandValueMap;

    icode::ModuleDescription& moduleDescription;
    icode::StringModulesMap& modulesMap;

    std::map<std::string, std::map<std::string, std::vector<llvm::Value*>>> params;
    llvm::Value* currentFunctionReturnValue;

    Console& console;

    ModuleContext(icode::ModuleDescription& moduleDescription, icode::StringModulesMap& modulesMap, Console& console);

    void clear();
};

#endif