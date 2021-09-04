#ifndef TRANSLATOR_GET_AND_SET_LLVM
#define TRANSLATOR_GET_AND_SET_LLVM

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

llvm::Value* getLLVMConstant(const ModuleContext& ctx, const icode::Operand& op);

llvm::Function* getLLVMFunction(const ModuleContext& ctx,
                                const std::string& functionName,
                                const icode::FunctionDescription& functionDescription);

icode::FunctionDescription getFunctionDescription(const ModuleContext& ctx,
                                                  const std::string& functionName,
                                                  const std::string& moduleName);

llvm::Value* getLLVMPointer(ModuleContext& ctx, const icode::Operand& op);

llvm::Value* getLLVMValue(ModuleContext& ctx, const icode::Operand& op);

void setLLVMValue(ModuleContext& ctx, const icode::Operand& op, llvm::Value* value);

llvm::Value* getLLVMPointerToPointer(ModuleContext& ctx, const icode::Operand& op);

#endif