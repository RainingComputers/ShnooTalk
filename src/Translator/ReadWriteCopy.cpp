#include "GetAndSetLLVM.hpp"
#include "ToLLVMType.hpp"

#include "ReadWriteCopy.hpp"

using namespace llvm;

void createPointer(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk CREATE_PTR to llvm ir */

    switch (e.op2.operandType)
    {
        case icode::TEMP_PTR:
            ctx.operandValueMap[e.op1.operandId] = ctx.operandValueMap[e.op2.operandId];
            break;
        case icode::TEMP_PTR_PTR:
        case icode::VAR:
        case icode::GBL_VAR:
        case icode::PTR:
        case icode::CALLEE_RET_VAL:
        case icode::RET_VALUE:
        case icode::STR_DATA:
            ctx.operandValueMap[e.op1.operandId] =
                ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, e.op2), dataTypeToLLVMType(ctx, icode::I64));
            break;
        default:
            ctx.console.internalBugError();
    }
}

void copy(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk EQUAL to llvm ir */

    Value* sourceValue = getLLVMValue(ctx, e.op2);
    setLLVMValue(ctx, e.op1, sourceValue);
}

void pointerAssign(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk PTR_ASSIGN to llvm ir */

    Value* destinationPointer = getLLVMPointerToPointer(ctx, e.op1);
    Value* sourceValue = getLLVMPointer(ctx, e.op2);

    if (sourceValue->getType()->getPointerElementType()->isArrayTy())
        sourceValue = ctx.builder->CreateBitCast(sourceValue, destinationPointer->getType()->getPointerElementType());

    ctx.builder->CreateStore(sourceValue, destinationPointer);
}

void read(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk READ to llvm ir */

    Value* sourcePointer = getLLVMPointer(ctx, e.op2);
    Value* sourceValue = ctx.builder->CreateLoad(sourcePointer);

    setLLVMValue(ctx, e.op1, sourceValue);
}

void write(ModuleContext& ctx, const icode::Entry& e)
{
    /* Converts ShnooTalk WRITE to llvm ir */

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
    /* Converts ShnooTalk ADDR_ADD and ADDR_MUL to llvm ir */

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
            ctx.console.internalBugError();
    }

    /* Store result llvm in map so it can be used by other llvm tranlations */
    setLLVMValue(ctx, e.op1, result);
}

void memCopy(ModuleContext& ctx, const icode::Entry& e)
{
    Value* dest = getLLVMPointer(ctx, e.op1);
    Value* src = getLLVMPointer(ctx, e.op2);
    Value* nbytes = getLLVMValue(ctx, e.op3);

    ctx.builder->CreateMemCpy(dest, MaybeAlign(), src, MaybeAlign(), nbytes);
}