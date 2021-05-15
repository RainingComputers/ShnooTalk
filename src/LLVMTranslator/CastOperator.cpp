#include "../log.hpp"
#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "CastOperator.hpp"

using namespace llvm;

Value* castToSignedInt(const ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isSignedInteger(e.op2.dtype))
        return ctx.builder->CreateSExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isUnsignedInteger(e.op2.dtype))
        return ctx.builder->CreateZExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isFloat(e.op2.dtype))
        return ctx.builder->CreateFPToSI(getLLVMValue(ctx, e.op2), destType);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* castToUnsignedInt(const ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isInteger(e.op2.dtype))
        return ctx.builder->CreateZExtOrTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::isFloat(e.op2.dtype))
        return ctx.builder->CreateFPToUI(getLLVMValue(ctx, e.op2), destType);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* castToFloatFromInt(const ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isSignedInteger(e.op2.dtype))
        return ctx.builder->CreateSIToFP(getLLVMValue(ctx, e.op2), destType);

    if (icode::isUnsignedInteger(e.op2.dtype))
        return ctx.builder->CreateUIToFP(getLLVMValue(ctx, e.op2), destType);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* castToFloatFromFloat(const ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::getDataTypeSize(e.op1.dtype) > icode::getDataTypeSize(e.op2.dtype))
        return ctx.builder->CreateFPExt(getLLVMValue(ctx, e.op2), destType);

    if (icode::getDataTypeSize(e.op1.dtype) < icode::getDataTypeSize(e.op2.dtype))
        return ctx.builder->CreateFPTrunc(getLLVMValue(ctx, e.op2), destType);

    if (icode::getDataTypeSize(e.op1.dtype) == icode::getDataTypeSize(e.op2.dtype))
        return getLLVMValue(ctx, e.op2);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* castToFloat(const ModuleContext& ctx, const icode::Entry& e, Type* destType)
{
    if (icode::isInteger(e.op2.dtype))
        return castToFloatFromInt(ctx, e, destType);

    if (icode::isFloat(e.op2.dtype))
        return castToFloatFromFloat(ctx, e, destType);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
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
    {
        miklog::internal_error(ctx.moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    setLLVMValue(ctx, e.op1, result);
}