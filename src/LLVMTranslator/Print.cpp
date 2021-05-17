#include "../log.hpp"

#include "GetAndSetLLVM.hpp"
#include "Print.hpp"
#include "ToLLVMType.hpp"

using namespace llvm;

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext)
{
    /* Setup global format strings */
    formatStringsContext.intFormatString =
      ctx.builder->CreateGlobalString("%d", "intFormatString", 0U, ctx.LLVMModule.get());

    formatStringsContext.uintFormatString =
      ctx.builder->CreateGlobalString("%u", "uintFormatString", 0U, ctx.LLVMModule.get());

    formatStringsContext.floatFormatString =
      ctx.builder->CreateGlobalString("%f", "floatFormatString", 0U, ctx.LLVMModule.get());

    formatStringsContext.newLineString = ctx.builder->CreateGlobalString("\n", "newln", 0U, ctx.LLVMModule.get());

    formatStringsContext.spaceString = ctx.builder->CreateGlobalString(" ", "space", 0U, ctx.LLVMModule.get());
}

Value* getFromatString(const ModuleContext& ctx,
                       const FormatStringsContext& formatStringsContext,
                       icode::DataType dtype)
{
    if (icode::isUnsignedInteger(dtype))
        return formatStringsContext.uintFormatString;

    if (icode::isSignedInteger(dtype))
        return formatStringsContext.intFormatString;

    if (icode::isFloat(dtype))
        return formatStringsContext.floatFormatString;

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

void setupPrintf(const ModuleContext& ctx, FormatStringsContext& formatStringsContext)
{

    /* Declare printf function */
    std::vector<Type*> args;
    args.push_back(Type::getInt8PtrTy(*ctx.context));
    FunctionType* printf_type = FunctionType::get(ctx.builder->getInt32Ty(), args, true);
    Function::Create(printf_type, Function::ExternalLinkage, "printf", ctx.LLVMModule.get());

    setupFormatStringsContext(ctx, formatStringsContext);
}

void callPrintf(const ModuleContext& ctx, Value* formatString, Value* value)
{
    /* Set up printf arguments*/
    std::vector<Value*> printArgs;
    printArgs.push_back(formatString);
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

    callPrintf(ctx, getFromatString(ctx, formatStringsContext, e.op1.dtype), value);
}

void printString(const ModuleContext& ctx, const icode::Entry& e)
{
    Value* str_value = getLLVMPointer(ctx, e.op1);

    callPrintf(ctx, str_value, nullptr);
}
