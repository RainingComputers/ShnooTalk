#include "llvm/IR/GlobalVariable.h"

#include "CreateSymbol.hpp"
#include "ToLLVMType.hpp"

using namespace llvm;

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    ctx.symbolNamePointersMap[name] =
      ctx.builder->CreateAlloca(typeDescriptionToLLVMType(ctx, typeDescription), nullptr, name);
}

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    GlobalVariable* global;
    Type* type = typeDescriptionToLLVMType(ctx, typeDescription);

    global = new GlobalVariable(*ctx.LLVMModule, type, false, GlobalVariable::CommonLinkage, nullptr, name);

    global->setInitializer(Constant::getNullValue(type));

    ctx.symbolNameGlobalsMap[name] = global;
}

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             const std::string& name,
                             llvm::Value* arg)
{
    if (!typeDescription.checkProperty(icode::IS_PTR))
    {
        Value* alloca = ctx.builder->CreateAlloca(typeDescriptionToLLVMType(ctx, typeDescription), nullptr, name);
        ctx.builder->CreateStore(arg, alloca);
        ctx.symbolNamePointersMap[name] = alloca;
    }
    else
    {
        ctx.symbolNamePointersMap[name] = arg;
        ctx.symbolNamePointerIntMap[name] = ctx.builder->CreatePtrToInt(arg, dataTypeToLLVMType(ctx, icode::I64));
    }
}