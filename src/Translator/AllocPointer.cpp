#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "AllocPointer.hpp"

using namespace llvm;

void allocPointer(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk ALLOC_PTR to llvm ir */

    Value* allocPointer = ctx.builder->CreateAlloca(dataTypeToLLVMType(ctx, e.op1.dtype));

    allocPointer = ctx.builder->CreatePtrToInt(allocPointer, dataTypeToLLVMType(ctx, icode::I64));

    setLLVMValue(ctx, e.op1, allocPointer);
}

void allocArrayPointer(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk ALLOC_ARRAY_PTR to llvm ir */

    Type* bytesArrayType = ArrayType::get(Type::getInt8Ty(*ctx.context), e.op2.val.bytes);

    Value* allocPointer = ctx.builder->CreateAlloca(bytesArrayType);
    allocPointer = ctx.builder->CreateBitCast(allocPointer, dataTypeToLLVMPointerType(ctx, e.op1.dtype));

    allocPointer = ctx.builder->CreatePtrToInt(allocPointer, dataTypeToLLVMType(ctx, icode::I64));

    setLLVMValue(ctx, e.op1, allocPointer);
}