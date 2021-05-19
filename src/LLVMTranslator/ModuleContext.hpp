
#ifndef MODULE_CONTEXT_HPP
#define MODULE_CONTEXT_HPP

#include <map>
#include <queue>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "../IntermediateRepresentation/All.hpp"

struct ModuleContext
{
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> LLVMModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    std::map<std::string, llvm::Value*> symbolNamePointersMap;
    std::map<std::string, llvm::Value*> symbolNamePointerIntMap;
    std::map<std::string, llvm::GlobalVariable*> symbolNameGlobalsMap;
    std::map<icode::Operand, llvm::Value*> operandValueMap;

    icode::ModuleDescription& moduleDescription;
    icode::StringModulesMap& externalModulesRef;

    std::map<std::string, std::map<std::string, std::vector<llvm::Value*>>> params;
    llvm::Value* currentFunctionReturnPointer;

    ModuleContext(icode::ModuleDescription& moduleDesc, icode::StringModulesMap& modulesMap);

    void clear();
};

#endif