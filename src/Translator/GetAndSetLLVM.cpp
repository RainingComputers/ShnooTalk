#include "../IntermediateRepresentation/All.hpp"
#include "ToLLVMType.hpp"

#include "GetAndSetLLVM.hpp"

using namespace llvm;

Value* getLLVMConstant(const ModuleContext& ctx, const icode::Operand& op)
{
    if (op.operandType == icode::ADDR)
        return ConstantInt::get(Type::getInt64Ty(*ctx.context), op.val.integer);

    if (icode::isInteger(op.dtype))
        return ConstantInt::get(dataTypeToLLVMType(ctx, op.dtype), op.val.integer);

    if (icode::isFloat(op.dtype))
        return ConstantFP::get(dataTypeToLLVMType(ctx, op.dtype), op.val.floating);

    ctx.console.internalBugError();
}

std::string getFullFunctionName(const std::string& functionName, const std::string& moduleName)
{
    if (functionName == "main")
        return functionName;

    return moduleName + "." + functionName;
}

Function* getLLVMFunction(const ModuleContext& ctx, const std::string& functionName, const std::string& moduleName)
{
    std::string fullFunctionName = getFullFunctionName(functionName, moduleName);

    if (auto* F = ctx.LLVMModule->getFunction(fullFunctionName))
        return F;

    FunctionType* functionType = funcDescriptionToLLVMType(ctx, ctx.modulesMap[moduleName].functions[functionName]);
    return Function::Create(functionType, Function::ExternalLinkage, fullFunctionName, *ctx.LLVMModule);
}

Value* getCalleeRetValuePointer(const ModuleContext& ctx, const icode::Operand& op)
{
    Value* calleeReturnValue = ctx.operandValueMap.at(op);
    Value* calleeReturnValuePointer = ctx.builder->CreateAlloca(calleeReturnValue->getType());
    ctx.builder->CreateStore(calleeReturnValue, calleeReturnValuePointer);

    return calleeReturnValuePointer;
}

Value* getLLVMValue(const ModuleContext& ctx, const icode::Operand& op)
{
    switch (op.operandType)
    {
        case icode::LITERAL:
        case icode::ADDR:
            return getLLVMConstant(ctx, op);
            return getLLVMConstant(ctx, op);
        case icode::VAR:
        case icode::GBL_VAR:
            return ctx.builder->CreateLoad(getLLVMPointer(ctx, op), op.name.c_str());
        case icode::RET_PTR:
        case icode::PTR:
            return ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, op), dataTypeToLLVMType(ctx, icode::I64));
        case icode::TEMP_PTR:
        case icode::TEMP:
        case icode::CALLEE_RET_VAL:
            return ctx.operandValueMap.at(op);

        default:
            ctx.console.internalBugError();
    }
}

Value* getLLVMPointer(const ModuleContext& ctx, const icode::Operand& op)
{
    switch (op.operandType)
    {
        case icode::PTR:
        case icode::VAR:
            return ctx.symbolNamePointersMap.at(op.name);
        case icode::GBL_VAR:
            return ctx.symbolNameGlobalsMap.at(op.name);
        case icode::RET_PTR:
            return ctx.currentFunctionReturnPointer;
        case icode::TEMP_PTR:
            return ctx.builder->CreateIntToPtr(getLLVMValue(ctx, op), dataTypeToLLVMPointerType(ctx, op.dtype));
        case icode::CALLEE_RET_VAL:
            return getCalleeRetValuePointer(ctx, op);
        case icode::STR_DATA:
            return ctx.operandGlobalStringMap.at(op.name);
        default:
            ctx.console.internalBugError();
    }
}

void setLLVMValue(ModuleContext& ctx, const icode::Operand& op, Value* value)
{
    switch (op.operandType)
    {
        case icode::TEMP:
        case icode::TEMP_PTR:
        case icode::CALLEE_RET_VAL:
            ctx.operandValueMap[op] = value;
            break;
        case icode::VAR:
        case icode::GBL_VAR:
            ctx.builder->CreateStore(value, getLLVMPointer(ctx, op));
            break;
        default:
            ctx.console.internalBugError();
    }
}