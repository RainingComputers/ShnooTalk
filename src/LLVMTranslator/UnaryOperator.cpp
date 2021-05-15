#include "GetAndSetLLVM.hpp"

#include "UnaryOperator.hpp"

using namespace llvm;

void bitwiseNot(ModuleContext& ctx, const icode::Entry& e)
{
    Value* result = ctx.builder->CreateNot(getLLVMValue(ctx, e.op2));

    setLLVMValue(ctx, e.op1, result);
}

void unaryMinus(ModuleContext& ctx, const icode::Entry& e)
{
    Value* result;

    if (icode::isInteger(e.op2.dtype))
        result = ctx.builder->CreateNeg(getLLVMValue(ctx, e.op2));
    else
        result = ctx.builder->CreateFNeg(getLLVMValue(ctx, e.op2));

    setLLVMValue(ctx, e.op1, result);
}