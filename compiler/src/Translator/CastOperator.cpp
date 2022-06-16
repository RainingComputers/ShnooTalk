#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "CastOperator.hpp"

using namespace llvm;

Value* castToSignedInt(ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isSignedInteger(e.op2.dtype))
        return ctx.builder->CreateSExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isUnsignedInteger(e.op2.dtype))
        return ctx.builder->CreateZExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isFloat(e.op2.dtype))
        return ctx.builder->CreateFPToSI(getLLVMValue(ctx, e.op2), destType);

    ctx.console.internalBugError();
}

Value* castToUnsignedInt(ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isInteger(e.op2.dtype))
        return ctx.builder->CreateZExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isFloat(e.op2.dtype))
        return ctx.builder->CreateFPToUI(getLLVMValue(ctx, e.op2), destType);

    ctx.console.internalBugError();
}

Value* castToFloatFromInt(ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isSignedInteger(e.op2.dtype))
        return ctx.builder->CreateSIToFP(getLLVMValue(ctx, e.op2), destType);

    if (icode::isUnsignedInteger(e.op2.dtype))
        return ctx.builder->CreateUIToFP(getLLVMValue(ctx, e.op2), destType);

    ctx.console.internalBugError();
}

Value* castToFloatFromFloat(ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::getDataTypeSize(e.op1.dtype) > icode::getDataTypeSize(e.op2.dtype))
        return ctx.builder->CreateFPExt(getLLVMValue(ctx, e.op2), destType);

    if (icode::getDataTypeSize(e.op1.dtype) < icode::getDataTypeSize(e.op2.dtype))
        return ctx.builder->CreateFPTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::getDataTypeSize(e.op1.dtype) == icode::getDataTypeSize(e.op2.dtype))
        return getLLVMValue(ctx, e.op2);

    ctx.console.internalBugError();
}

Value* castToFloat(ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isInteger(e.op2.dtype))
        return castToFloatFromInt(ctx, e, destType);

    if (icode::isFloat(e.op2.dtype))
        return castToFloatFromFloat(ctx, e, destType);

    ctx.console.internalBugError();
}

void castOperator(ModuleContext& ctx, const icode::Entry& e)
{
    Type* destType = dataTypeToLLVMType(ctx, e.op1.dtype);
    Value* result;

    if (icode::isSignedInteger(e.op1.dtype))
        result = castToSignedInt(ctx, e, destType);
    else if (icode::isUnsignedInteger(e.op1.dtype))
        result = castToUnsignedInt(ctx, e, destType);
    else if (icode::isFloat(e.op1.dtype))
        result = castToFloat(ctx, e, destType);
    else
        ctx.console.internalBugError();

    setLLVMValue(ctx, e.op1, result);
}

void pointerCastOperator(ModuleContext& ctx, const icode::Entry& e)
{
    Value* castedPointer;
    Type* destinationLLVMType = dataTypeToLLVMPointerType(ctx, e.op1.dtype);

    if (e.op2.operandType == icode::TEMP || e.op2.operandType == icode::LITERAL)
        castedPointer = ctx.builder->CreateIntToPtr(getLLVMValue(ctx, e.op2), destinationLLVMType);
    else
        castedPointer = ctx.builder->CreateBitCast(getLLVMPointer(ctx, e.op2), destinationLLVMType);

    setLLVMValue(ctx, e.op1, castedPointer);
}

void addrOperator(ModuleContext& ctx, const icode::Entry& e)
{
    Type* destinationLLVMType = dataTypeToLLVMType(ctx, e.op1.dtype);

    setLLVMValue(ctx, e.op1, ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, e.op2), destinationLLVMType));
}