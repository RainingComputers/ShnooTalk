#include "llvm/IR/GlobalVariable.h"

#include "CreateSymbol.hpp"
#include "ToLLVMType.hpp"

using namespace llvm;

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    Type* type = typeDescriptionToAllocaLLVMType(ctx, typeDescription);

    GlobalVariable* global =
        new GlobalVariable(*ctx.LLVMModule, type, false, GlobalVariable::PrivateLinkage, nullptr, name);

    global->setInitializer(Constant::getNullValue(type));

    ctx.symbolNameGlobalsMap[name] = global;
}

void createGlobalString(ModuleContext& ctx, const std::string& key, const std::string& str)
{

    Constant* strConstant = ConstantDataArray::getString(*ctx.context, str);

    GlobalVariable* globalString = new GlobalVariable(*ctx.LLVMModule,
                                                      strConstant->getType(),
                                                      true,
                                                      GlobalValue::ExternalLinkage,
                                                      strConstant,
                                                      key);

    ctx.operandGlobalStringMap[key] = globalString;
}

Value* createExternGlobalString(ModuleContext& ctx, const std::string& key)
{
    GlobalVariable* globalString = new GlobalVariable(*ctx.LLVMModule,
                                                      dataTypeToLLVMPointerType(ctx, icode::I8),
                                                      true,
                                                      GlobalVariable::ExternalLinkage,
                                                      nullptr,
                                                      key,
                                                      nullptr,
                                                      GlobalVariable::NotThreadLocal,
                                                      llvm::None,
                                                      true);
    ctx.operandGlobalStringMap[key] = globalString;
    return globalString;
}

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             const std::string& name,
                             llvm::Value* arg)
{
    if (typeDescription.isPassedByReference())
    {
        ctx.symbolNamePointersMap[name] = arg;
    }
    else
    {
        Value* alloca =
            ctx.builder->CreateAlloca(typeDescriptionToAllocaLLVMType(ctx, typeDescription), nullptr, name);

        Value* castedArg = arg;

        /* When struct pointer is passed to function, the type will be different
            (see typeDescriptionToLLVMType() in getAndSetLLVM.cpp) */
        if (typeDescription.isStruct())
            castedArg = ctx.builder->CreateBitCast(arg, dataTypeToLLVMPointerType(ctx, icode::I8), name);

        ctx.builder->CreateStore(castedArg, alloca);
        ctx.symbolNamePointersMap[name] = alloca;
    }
}

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    ctx.symbolNamePointersMap[name] =
        ctx.builder->CreateAlloca(typeDescriptionToAllocaLLVMType(ctx, typeDescription), nullptr, name);
}
