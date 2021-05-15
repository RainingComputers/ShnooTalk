#ifndef TO_LLVM_TYPE_HPP
#define TO_LLVM_TYPE_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

#include "ModuleContext.hpp"

llvm::Type* dataTypeToLLVMType(const ModuleContext& ctx, const icode::DataType dtype);
llvm::Type* dataTypeToLLVMPointerType(const ModuleContext& ctx, const icode::DataType dtype);
llvm::Type* variableDescriptionToLLVMType(const ModuleContext& ctx, const icode::VariableDescription& variableDesc);
llvm::FunctionType* funcDescriptionToLLVMType(const ModuleContext& ctx,
                                              const icode::FunctionDescription& functionDesc);

#endif