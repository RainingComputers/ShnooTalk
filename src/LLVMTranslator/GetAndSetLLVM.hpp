#ifndef GET_LLVM_HPP
#define GET_LLVM_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

llvm::Value* getLLVMConstant(const ModuleContext& ctx, const icode::Operand& op);
llvm::Function* getLLVMFunction(const ModuleContext& ctx,
                                const std::string& functionName,
                                const std::string& moduleName);
llvm::Value* getCalleeRetValuePointer(const ModuleContext& ctx, const icode::Operand& op);
llvm::Value* getLLVMPointer(const ModuleContext& ctx, const icode::Operand& op);
llvm::Value* getLLVMValue(const ModuleContext& ctx, const icode::Operand& op);
void setLLVMValue(ModuleContext& ctx, const icode::Operand& op, llvm::Value* value);

#endif