#include "Print.hpp"
#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

using namespace llvm;

void setupPrintf(const ModuleContext& ctx)
{
    /* Declare printf function */
    std::vector<Type*> args;
    args.push_back(Type::getInt8PtrTy(*ctx.context));
    FunctionType* printfFunctionType = FunctionType::get(ctx.builder->getInt32Ty(), args, true);
    Function::Create(printfFunctionType, Function::ExternalLinkage, "printf", ctx.LLVMModule.get());
}

void callPrintf(const ModuleContext& ctx, Value* formatString, Value* value)
{
    /* Set up printf arguments*/
    std::vector<Value*> printArgs;

    Value* formatStringCasted = ctx.builder->CreateBitCast(formatString, dataTypeToLLVMPointerType(ctx, icode::UI8));
    printArgs.push_back(formatStringCasted);

    if (value)
        printArgs.push_back(value);

    /* Call printf */
    ctx.builder->CreateCall(ctx.LLVMModule->getFunction("printf"), printArgs);
}

void print(const ModuleContext& ctx, const FormatStringsContext& formatStringsContext, const icode::Entry& e)
{
    Value* value = getLLVMValue(ctx, e.op1);

    /* Cast value to double if float, int64 if int */
    if (icode::isFloat(e.op1.dtype))
        value = ctx.builder->CreateFPCast(value, dataTypeToLLVMType(ctx, icode::F64));
    else
        value = ctx.builder->CreateSExt(value, dataTypeToLLVMType(ctx, icode::I64));

    callPrintf(ctx, getFromatStringFromDataType(formatStringsContext, e.op1.dtype), value);
}

void printString(const ModuleContext& ctx, const icode::Entry& e)
{
    Value* str_value = getLLVMPointer(ctx, e.op1);

    callPrintf(ctx, str_value, nullptr);
}
