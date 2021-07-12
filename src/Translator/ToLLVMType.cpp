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
            return Type::getInt32Ty(*ctx.context);
        case icode::I64:
        case icode::UI64:
        case icode::AUTO_INT:
            return Type::getInt64Ty(*ctx.context);
        case icode::F32:
            return Type::getFloatTy(*ctx.context);
        case icode::F64:
        case icode::AUTO_FLOAT:
            return Type::getDoubleTy(*ctx.context);
        case icode::VOID:
            return Type::getVoidTy(*ctx.context);
        default:
            ctx.console.internalBugError();
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
            return Type::getInt32PtrTy(*ctx.context);
        case icode::I64:
        case icode::UI64:
        case icode::AUTO_INT:
            return Type::getInt64PtrTy(*ctx.context);
        case icode::F32:
            return Type::getFloatPtrTy(*ctx.context);
        case icode::F64:
        case icode::AUTO_FLOAT:
            return Type::getDoublePtrTy(*ctx.context);
        default:
            ctx.console.internalBugError();
    }
}

Type* nonPointerTypeDescriptionToLLVMType(const ModuleContext& ctx, const icode::TypeDescription& typeDescription)
{
    if (typeDescription.dimensions.size() > 0 || typeDescription.dtype == icode::STRUCT)
        return ArrayType::get(Type::getInt8Ty(*ctx.context), typeDescription.size);

    return dataTypeToLLVMType(ctx, typeDescription.dtype);
}

Type* typeDescriptionToLLVMType(const ModuleContext& ctx, const icode::TypeDescription& typeDescription)
{
    if (typeDescription.checkProperty(icode::IS_PTR))
        return nonPointerTypeDescriptionToLLVMType(ctx, typeDescription)->getPointerTo();

    return nonPointerTypeDescriptionToLLVMType(ctx, typeDescription);
}

FunctionType* funcDescriptionToLLVMType(const ModuleContext& ctx, const icode::FunctionDescription& functionDesc)
{
    std::vector<Type*> parameterTypes;

    for (std::string paramString : functionDesc.parameters)
    {
        Type* type = typeDescriptionToLLVMType(ctx, functionDesc.symbols.at(paramString));
        parameterTypes.push_back(type);
    }

    FunctionType* FT =
      FunctionType::get(typeDescriptionToLLVMType(ctx, functionDesc.functionReturnType), parameterTypes, false);

    return FT;
}