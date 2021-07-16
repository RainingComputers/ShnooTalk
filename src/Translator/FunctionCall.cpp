#include "GetAndSetLLVM.hpp"

#include "FunctionCall.hpp"

using namespace llvm;

void call(ModuleContext& ctx, const icode::Entry& e)
{
    const std::string& functionName = e.op2.name;
    const std::string& moduleName = e.op3.name;

    Value* result =
      ctx.builder->CreateCall(getLLVMFunction(ctx, functionName, e.op3.name), ctx.params[moduleName][functionName]);

    setLLVMValue(ctx, e.op1, result);

    ctx.params[moduleName][functionName].clear();
}

void ret(const ModuleContext& ctx, const icode::Entry& e, icode::DataType dtype)
{
    if (dtype == icode::VOID)
        ctx.builder->CreateRetVoid();
    else
        ctx.builder->CreateRet(ctx.builder->CreateLoad(ctx.currentFunctionReturnValue));
}

void pushParam(ModuleContext& ctx, const icode::Entry& e, Value* value)
{
    const std::string& functionName = e.op2.name;
    const std::string& moduleName = e.op3.name;

    ctx.params[moduleName][functionName].push_back(value);
}

void pass(ModuleContext& ctx, const icode::Entry& e)
{
    pushParam(ctx, e, getLLVMValue(ctx, e.op1));
}

void passPointer(ModuleContext& ctx, const icode::Entry& e)
{
    pushParam(ctx, e, getLLVMPointer(ctx, e.op1));
}