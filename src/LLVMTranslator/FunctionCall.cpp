#include "GetAndSetLLVM.hpp"

#include "FunctionCall.hpp"

using namespace llvm;

void call(ModuleContext& ctx, const icode::Entry& e)
{
    Value* result = ctx.builder->CreateCall(getLLVMFunction(ctx, e.op2.name, e.op3.name), ctx.params);

    setLLVMValue(ctx, e.op1, result);

    ctx.params.clear();
}

void ret(const ModuleContext& ctx, const icode::Entry& e, icode::DataType dtype)
{
    if (dtype == icode::VOID)
        ctx.builder->CreateRetVoid();
    else
        ctx.builder->CreateRet(ctx.builder->CreateLoad(ctx.currentFunctionReturnPointer));
}

void pass(ModuleContext& ctx, const icode::Entry& e)
{
    ctx.params.push_back(getLLVMValue(ctx, e.op1));
}

void passPointer(ModuleContext& ctx, const icode::Entry& e)
{
    ctx.params.push_back(getLLVMPointer(ctx, e.op1));
}