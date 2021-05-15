#include "../log.hpp"
#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "ReadWriteCopy.hpp"

using namespace llvm;

void createPointer(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts mikuro CREATE_PTR to llvm ir */

    switch (e.op2.operandType)
    {
        case icode::TEMP_PTR:
            ctx.operandValueMap[e.op1] = ctx.operandValueMap[e.op2];
            break;
        case icode::VAR:
        case icode::GBL_VAR:
            ctx.operandValueMap[e.op1] =
              ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, e.op2), dataTypeToLLVMType(ctx, icode::I64));
            break;
        case icode::PTR:
            ctx.operandValueMap[e.op1] = ctx.symbolNamePointerIntMap[e.op2.name];
            break;
        case icode::CALLEE_RET_VAL:
            ctx.operandValueMap[e.op1] = getCalleeRetValuePointer(ctx, e.op2);
            break;
        case icode::RET_PTR:
            ctx.operandValueMap[e.op1] = ctx.currentFunctionReturnPointer;
            break;
        default:
            miklog::internal_error(ctx.moduleDescription.name);
            throw miklog::internal_bug_error();
    }
}

void copy(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts mikuro EQUAL to llvm ir */

    Value* sourceValue = getLLVMValue(ctx, e.op2);
    setLLVMValue(ctx, e.op1, sourceValue);
}

void read(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts mikuro READ to llvm ir */

    Value* sourcePointer = getLLVMPointer(ctx, e.op2);
    Value* sourceValue = ctx.builder->CreateLoad(sourcePointer);

    setLLVMValue(ctx, e.op1, sourceValue);
}

void write(const ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts mikuro WRITE to llvm ir */

    Value* destinationPointer = getLLVMPointer(ctx, e.op1);
    Value* sourceValue = getLLVMValue(ctx, e.op2);

    ctx.builder->CreateStore(sourceValue, destinationPointer);
}

llvm::Value* ensureI64(const ModuleContext& ctx, llvm::Value* value)
{
    if (value->getType() == dataTypeToLLVMType(ctx, icode::I64))
        return value;

    return ctx.builder->CreateZExtOrTrunc(value, dataTypeToLLVMType(ctx, icode::I64));
}

void addressBinaryOperator(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts mikuro ADDR_ADD and ADDR_MUL to llvm ir */

    Value* result;
    Value* LHS = getLLVMValue(ctx, e.op2);
    Value* RHS = getLLVMValue(ctx, e.op3);

    switch (e.opcode)
    {
        case icode::ADDR_ADD:
            result = ctx.builder->CreateNUWAdd(LHS, RHS);
            break;
        case icode::ADDR_MUL:
            result = ctx.builder->CreateNUWMul(ensureI64(ctx, LHS), RHS);
            break;
        default:
            miklog::internal_error(ctx.moduleDescription.name);
            throw miklog::internal_bug_error();
    }

    /* Store result llvm in map so it can be used by other llvm tranlations */
    setLLVMValue(ctx, e.op1, result);
}