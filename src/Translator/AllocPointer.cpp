#include "ToLLVMType.hpp"
#include "GetAndSetLLVM.hpp"

#include "AllocPointer.hpp"

using namespace llvm;

void allocPointer(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk ALLOC_PTR to llvm ir */

    Value* allocPointer = ctx.builder->CreateAlloca(dataTypeToLLVMType(ctx, e.op1.dtype));
    setLLVMValue(ctx, e.op1, allocPointer);
}