#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "FunctionCall.hpp"

using namespace llvm;

void pushParam(ModuleContext& ctx, const icode::Entry& e, Value* value)
{
    const std::string& functionName = e.op2.name;

    ctx.params[functionName].push_back(value);
}

Value* createCalleeReturnValue(const ModuleContext& ctx, const icode::TypeDescription& returnType)
{
    if (returnType.dtype == icode::VOID)
        return nullptr;

    return ctx.builder->CreateAlloca(typeDescriptionToLLVMType(ctx, returnType));
}

void call(ModuleContext& ctx, const icode::Entry& e)
{
    const std::string& functionName = e.op2.name;

    const icode::TypeDescription& returnType = getFunctionReturnType(ctx, functionName, e.op3.name);
    Value* calleeReturnValuePointer = createCalleeReturnValue(ctx, returnType);

    /* If the function returns a struct or an array, pass return value by reference,
        (as the last argument) else return the return value normally */

    if (!returnType.isStructOrArray())
    {
        Value* result =
            ctx.builder->CreateCall(getLLVMFunction(ctx, functionName, e.op3.name), ctx.params[functionName]);

        if (returnType.dtype != icode::VOID)
            ctx.builder->CreateStore(result, calleeReturnValuePointer);
    }
    else
    {
        pushParam(ctx, e, calleeReturnValuePointer);
        ctx.builder->CreateCall(getLLVMFunction(ctx, functionName, e.op3.name), ctx.params[functionName]);
    }

    setLLVMValue(ctx, e.op1, calleeReturnValuePointer);

    ctx.params[functionName].clear();
}

void ret(const ModuleContext& ctx, const icode::Entry& e, const icode::TypeDescription& functionReturnType)
{
    if (functionReturnType.isStructOrArray() || functionReturnType.dtype == icode::VOID)
        ctx.builder->CreateRetVoid();
    else
        ctx.builder->CreateRet(ctx.builder->CreateLoad(ctx.currentFunctionReturnValue));
}

void pass(ModuleContext& ctx, const icode::Entry& e)
{
    pushParam(ctx, e, getLLVMValue(ctx, e.op1));
}

void passPointer(ModuleContext& ctx, const icode::Entry& e)
{
    pushParam(ctx, e, getLLVMPointer(ctx, e.op1));
}