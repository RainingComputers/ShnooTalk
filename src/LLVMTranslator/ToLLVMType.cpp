#include "../log.hpp"

#include "ToLLVMType.hpp"

using namespace llvm;

Type* dataTypeToLLVMType(const ModuleContext& ctx, const icode::DataType dtype)
{

    switch (dtype)
    {
        case icode::I8:
        case icode::UI8:
            return Type::getInt8Ty(*ctx.context);
        case icode::I16:
        case icode::UI16:
            return Type::getInt16Ty(*ctx.context);
        case icode::I32:
        case icode::UI32:
        case icode::INT:
            return Type::getInt32Ty(*ctx.context);
        case icode::I64:
        case icode::UI64:
            return Type::getInt64Ty(*ctx.context);
        case icode::F32:
        case icode::FLOAT:
            return Type::getFloatTy(*ctx.context);
        case icode::F64:
            return Type::getDoubleTy(*ctx.context);
        case icode::VOID:
            return Type::getVoidTy(*ctx.context);
        default:
            miklog::internal_error(ctx.moduleDescription.name);
            throw miklog::internal_bug_error();
    }
}

Type* dataTypeToLLVMPointerType(const ModuleContext& ctx, const icode::DataType dtype)
{
    switch (dtype)
    {
        case icode::I8:
        case icode::UI8:
        case icode::STRUCT:
            return Type::getInt8PtrTy(*ctx.context);
        case icode::I16:
        case icode::UI16:
            return Type::getInt16PtrTy(*ctx.context);
        case icode::I32:
        case icode::UI32:
        case icode::INT:
            return Type::getInt32PtrTy(*ctx.context);
        case icode::I64:
        case icode::UI64:
            return Type::getInt64PtrTy(*ctx.context);
        case icode::F32:
        case icode::FLOAT:
            return Type::getFloatPtrTy(*ctx.context);
        case icode::F64:
            return Type::getDoublePtrTy(*ctx.context);
        default:
            miklog::internal_error(ctx.moduleDescription.name);
            throw miklog::internal_bug_error();
    }
}

Type* variableDescriptionToLLVMType(const ModuleContext& ctx, const icode::VariableDescription& variableDesc)
{
    if (variableDesc.checkProperty(icode::IS_PTR))
        return dataTypeToLLVMPointerType(ctx, variableDesc.dtype);

    if (variableDesc.dimensions.size() > 0 || variableDesc.dtype == icode::STRUCT)
        return ArrayType::get(Type::getInt8Ty(*ctx.context), variableDesc.size);

    return dataTypeToLLVMType(ctx, variableDesc.dtype);
}

FunctionType* funcDescriptionToLLVMType(const ModuleContext& ctx, const icode::FunctionDescription& functionDesc)
{
    std::vector<Type*> parameterTypes;

    /* Set the types vector */
    for (std::string paramString : functionDesc.parameters)
    {
        Type* type = variableDescriptionToLLVMType(ctx, functionDesc.symbols.at(paramString));
        parameterTypes.push_back(type);
    }

    /* Setup llvm function */
    FunctionType* FT = FunctionType::get(variableDescriptionToLLVMType(ctx, functionDesc.functionReturnDescription),
                                         parameterTypes,
                                         false);

    return FT;
}