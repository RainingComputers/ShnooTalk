#include "../IntermediateRepresentation/All.hpp"
#include "CreateSymbol.hpp"
#include "ToLLVMType.hpp"

#include "GetAndSetLLVM.hpp"

using namespace llvm;

Value* getLLVMConstant(const ModuleContext& ctx, const icode::Operand& op)
{
    if (op.operandType == icode::BYTES)
        return ConstantInt::get(Type::getInt64Ty(*ctx.context), op.val.integer);

    if (icode::isInteger(op.dtype))
        return ConstantInt::get(dataTypeToLLVMType(ctx, op.dtype), op.val.integer);

    if (icode::isFloat(op.dtype))
        return ConstantFP::get(dataTypeToLLVMType(ctx, op.dtype), op.val.floating);

    ctx.console.internalBugError();
}

icode::FunctionDescription getFunctionDescription(const ModuleContext& ctx,
                                                  const std::string& functionName,
                                                  const std::string& moduleName)
{
    icode::FunctionDescription functionDescription;

    /* For forward declared functions */
    if (ctx.moduleDescription.getExternFunction(functionName, functionDescription))
        return functionDescription;

    icode::ModuleDescription functionModule = ctx.modulesMap.at(moduleName);

    if (functionModule.getFunction(functionName, functionDescription))
        return functionDescription;

    if (functionModule.getExternFunction(functionName, functionDescription))
        return functionDescription;

    ctx.console.internalBugError();
}

Function* getLLVMFunction(const ModuleContext& ctx,
                          const std::string& functionName,
                          const icode::FunctionDescription& functionDescription)
{
    if (auto* F = ctx.LLVMModule->getFunction(functionName))
        return F;

    FunctionType* functionType = funcDescriptionToLLVMType(ctx, functionDescription);
    return Function::Create(functionType, Function::ExternalLinkage, functionName, *ctx.LLVMModule);
}

Value* getStringDataPointer(ModuleContext& ctx, const icode::Operand& op)
{
    auto result = ctx.operandGlobalStringMap.find(op.name);

    if (result != ctx.operandGlobalStringMap.end())
        return result->second;

    return createExternGlobalString(ctx, op.name);
}

Value* getLLVMPointer(ModuleContext& ctx, const icode::Operand& op)
{
    switch (op.operandType)
    {
        case icode::GBL_VAR:
            return ctx.symbolNameGlobalsMap.at(op.name);
        case icode::VAR:
            return ctx.symbolNamePointersMap.at(op.name);
        case icode::PTR:
            return ctx.builder->CreateLoad(ctx.symbolNamePointersMap.at(op.name));
        case icode::RET_VALUE:
            return ctx.currentFunctionReturnValue;
        case icode::TEMP_PTR:
            return ctx.builder->CreateIntToPtr(getLLVMValue(ctx, op), dataTypeToLLVMPointerType(ctx, op.dtype));
        case icode::TEMP_PTR_PTR:
            return ctx.builder->CreateLoad(getLLVMPointerToPointer(ctx, op));
        case icode::CALLEE_RET_VAL:
            return ctx.operandValueMap.at(op.operandId);
        case icode::CALLEE_RET_PTR:
            return ctx.builder->CreateLoad(ctx.operandValueMap.at(op.operandId));
        case icode::STR_DATA:
            return getStringDataPointer(ctx, op);
        default:
            ctx.console.internalBugError();
    }
}

Value* getLLVMValue(ModuleContext& ctx, const icode::Operand& op)
{
    switch (op.operandType)
    {
        case icode::LITERAL:
        case icode::BYTES:
            return getLLVMConstant(ctx, op);
        case icode::VAR:
        case icode::GBL_VAR:
        case icode::CALLEE_RET_VAL:
            return ctx.builder->CreateLoad(getLLVMPointer(ctx, op), op.name.c_str());
        case icode::PTR:
        case icode::CALLEE_RET_PTR:
        case icode::TEMP_PTR_PTR:
            return ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, op), dataTypeToLLVMType(ctx, icode::I64));
        case icode::TEMP_PTR:
        case icode::TEMP:
            return ctx.operandValueMap.at(op.operandId);
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
        case icode::TEMP_PTR_PTR:
        case icode::CALLEE_RET_VAL:
        case icode::CALLEE_RET_PTR:
            ctx.operandValueMap[op.operandId] = value;
            break;
        case icode::VAR:
        case icode::GBL_VAR:
        case icode::RET_VALUE:
            ctx.builder->CreateStore(value, getLLVMPointer(ctx, op));
            break;
        default:
            ctx.console.internalBugError();
    }
}

Value* getLLVMPointerToPointer(ModuleContext& ctx, const icode::Operand& op)
{
    switch (op.operandType)
    {
        case icode::PTR:
            return ctx.symbolNamePointersMap.at(op.name);
        case icode::RET_PTR:
            return ctx.currentFunctionReturnValue;
        case icode::TEMP_PTR_PTR:
            return ctx.builder->CreateIntToPtr(ctx.operandValueMap[op.operandId],
                                               dataTypeToLLVMPointerType(ctx, op.dtype)->getPointerTo());
        default:
            ctx.console.internalBugError();
    }
}
