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
    icode::ModuleDescription& functionModule = ctx.modulesMap.at(moduleName);

    icode::FunctionDescription functionDescription;

    if (functionModule.getFunction(functionName, functionDescription))
        return functionDescription;

    if (functionModule.getExternFunction(functionName, functionDescription))
        return functionDescription;

    ctx.console.internalBugError();
}

Function* getLLVMFunction(const ModuleContext& ctx, const std::string& functionName, const std::string& moduleName)
{
    if (auto* F = ctx.LLVMModule->getFunction(functionName))
        return F;

    FunctionType* functionType = funcDescriptionToLLVMType(ctx, getFunctionDescription(ctx, functionName, moduleName));
    return Function::Create(functionType, Function::ExternalLinkage, functionName, *ctx.LLVMModule);
}

icode::TypeDescription getFunctionReturnType(const ModuleContext& ctx,
                                             const std::string& functionName,
                                             const std::string& moduleName)
{
    const icode::FunctionDescription& functionDescription = getFunctionDescription(ctx, functionName, moduleName);
    return functionDescription.functionReturnType;
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
        case icode::PTR:
        case icode::VAR:
            return ctx.symbolNamePointersMap.at(op.name);
        case icode::GBL_VAR:
            return ctx.symbolNameGlobalsMap.at(op.name);
        case icode::RET_VALUE:
            return ctx.currentFunctionReturnValue;
        case icode::TEMP_PTR:
            return ctx.builder->CreateIntToPtr(getLLVMValue(ctx, op), dataTypeToLLVMPointerType(ctx, op.dtype));
        case icode::CALLEE_RET_VAL:
            return ctx.operandValueMap.at(op);
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
            return ctx.builder->CreatePtrToInt(getLLVMPointer(ctx, op), dataTypeToLLVMType(ctx, icode::I64));
        case icode::TEMP_PTR:
        case icode::TEMP:
            return ctx.operandValueMap.at(op);
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
        case icode::RET_VALUE:
            ctx.builder->CreateStore(value, getLLVMPointer(ctx, op));
            break;
        default:
            ctx.console.internalBugError();
    }
}