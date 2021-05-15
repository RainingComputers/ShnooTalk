#include "../log.hpp"
#include "GetAndSetLLVM.hpp"

#include "BinaryOperator.hpp"

using namespace llvm;

Value* add(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro ADD to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateNSWAdd(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateNUWAdd(LHS, RHS);

    if (icode::isFloat(dtype))
        return ctx.builder->CreateFAdd(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* subtract(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro SUB to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateNSWSub(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateNUWSub(LHS, RHS);

    if (icode::isFloat(dtype))
        return ctx.builder->CreateFSub(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* multiply(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro MUL to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateNSWMul(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateNUWMul(LHS, RHS);

    if (icode::isFloat(dtype))
        return ctx.builder->CreateFMul(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* divide(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro DIV to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateSDiv(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateUDiv(LHS, RHS);

    if (icode::isFloat(dtype))
        return ctx.builder->CreateFDiv(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* remainder(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro MOD to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateSRem(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateURem(LHS, RHS);

    if (icode::isFloat(dtype))
        return ctx.builder->CreateFRem(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* rightShift(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro RSH to llvm ir */

    if (icode::isSignedInteger(dtype))
        return ctx.builder->CreateAShr(LHS, RHS);

    if (icode::isUnsignedInteger(dtype))
        return ctx.builder->CreateLShr(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* leftShift(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro LSH to llvm ir */

    if (icode::isInteger(dtype))
        return ctx.builder->CreateShl(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* bitwiseAnd(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro BWA to llvm ir */

    if (icode::isInteger(dtype))
        return ctx.builder->CreateAnd(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* bitwiseOr(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro BWO to llvm ir */

    if (icode::isInteger(dtype))
        return ctx.builder->CreateOr(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* bitwiseXor(const ModuleContext& ctx, Value* LHS, Value* RHS, icode::DataType dtype)
{
    /* Converts mikuro BWX to llvm ir */

    if (icode::isInteger(dtype))
        return ctx.builder->CreateXor(LHS, RHS);

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* getBinaryOperatorValue(const ModuleContext& ctx, const icode::Entry& e)
{
    Value* LHS = getLLVMValue(ctx, e.op2);
    Value* RHS = getLLVMValue(ctx, e.op3);
    icode::DataType dtype = e.op1.dtype;

    switch (e.opcode)
    {
        case icode::ADD:
            return add(ctx, LHS, RHS, dtype);
        case icode::SUB:
            return subtract(ctx, LHS, RHS, dtype);
        case icode::MUL:
            return multiply(ctx, LHS, RHS, dtype);
        case icode::DIV:
            return divide(ctx, LHS, RHS, dtype);
        case icode::MOD:
            return remainder(ctx, LHS, RHS, dtype);
        case icode::RSH:
            return rightShift(ctx, LHS, RHS, dtype);
        case icode::LSH:
            return leftShift(ctx, LHS, RHS, dtype);
        case icode::BWA:
            return bitwiseAnd(ctx, LHS, RHS, dtype);
        case icode::BWO:
            return bitwiseOr(ctx, LHS, RHS, dtype);
        case icode::BWX:
            return bitwiseXor(ctx, LHS, RHS, dtype);
        default:
            miklog::internal_error(ctx.moduleDescription.name);
            throw miklog::internal_bug_error();
    }
}

void binaryOperator(ModuleContext& ctx, const icode::Entry& e)
{
    setLLVMValue(ctx, e.op1, getBinaryOperatorValue(ctx, e));
}