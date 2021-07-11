#ifndef TRANSLATOR_TO_LLVM_TYPE
#define TRANSLATOR_TO_LLVM_TYPE

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

#include "ModuleContext.hpp"

llvm::Type* dataTypeToLLVMType(const ModuleContext& ctx, const icode::DataType dtype);
llvm::Type* dataTypeToLLVMPointerType(const ModuleContext& ctx, const icode::DataType dtype);
llvm::Type* typeDescriptionToLLVMType(const ModuleContext& ctx, const icode::TypeDescription& typeDescription);
llvm::FunctionType* funcDescriptionToLLVMType(const ModuleContext& ctx,
                                              const icode::FunctionDescription& functionDesc);

#endif