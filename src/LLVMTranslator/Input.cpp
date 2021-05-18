#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "Input.hpp"

using namespace llvm;

void setupScanf(const ModuleContext& ctx)
{
    /* Declare printf function */
    std::vector<Type*> args;
    args.push_back(Type::getInt8PtrTy(*ctx.context));
    FunctionType* scanfFunctionType = FunctionType::get(ctx.builder->getInt32Ty(), args, true);
    Function::Create(scanfFunctionType, Function::ExternalLinkage, "scanf", ctx.LLVMModule.get());
}

void callScanf(const ModuleContext& ctx, Value* formatString, Value* value)
{
    /* Set up printf arguments*/
    std::vector<Value*> printArgs;
    printArgs.push_back(formatString);
    if (value)
        printArgs.push_back(value);

    /* Call printf */
    ctx.builder->CreateCall(ctx.LLVMModule->getFunction("scanf"), printArgs);
}

void input(const ModuleContext& ctx, const FormatStringsContext& formatStringsContext, const icode::Entry& e)
{
    callScanf(ctx, getFromatString(ctx, formatStringsContext, e.op1.dtype), getLLVMPointer(ctx, e.op1));
}

void inputString(const ModuleContext& ctx, const icode::Entry& e) 
{
    callScanf(ctx, getFormatStringForStringInput(ctx, e.op2.val.size), getLLVMPointer(ctx, e.op1));
}
