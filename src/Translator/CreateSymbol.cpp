#include "llvm/IR/GlobalVariable.h"

#include "CreateSymbol.hpp"
#include "ToLLVMType.hpp"

using namespace llvm;

void createGlobalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    Type* type = typeDescriptionToLLVMType(ctx, typeDescription);

    GlobalVariable* global =
      new GlobalVariable(*ctx.LLVMModule, type, false, GlobalVariable::PrivateLinkage, nullptr, name);

    global->setInitializer(Constant::getNullValue(type));

    ctx.symbolNameGlobalsMap[name] = global;
}

void createGlobalString(ModuleContext& ctx, const std::string& key, const std::string& str)
{

    Constant* strConstant = ConstantDataArray::getString(*ctx.context, str);

    GlobalVariable* globalString =
      new GlobalVariable(*ctx.LLVMModule, strConstant->getType(), true, GlobalValue::ExternalLinkage, strConstant, key);

    ctx.operandGlobalStringMap[key] = globalString;
}

void createFunctionParameter(ModuleContext& ctx,
                             const icode::TypeDescription& typeDescription,
                             const std::string& name,
                             llvm::Value* arg)
{
    if (!typeDescription.isPointer())
    {
        Value* alloca = ctx.builder->CreateAlloca(typeDescriptionToLLVMType(ctx, typeDescription), nullptr, name);
        ctx.builder->CreateStore(arg, alloca);
        ctx.symbolNamePointersMap[name] = alloca;
    }
    else
    {
        ctx.symbolNamePointersMap[name] = arg;
    }
}

void createLocalSymbol(ModuleContext& ctx, const icode::TypeDescription& typeDescription, const std::string& name)
{
    ctx.symbolNamePointersMap[name] =
      ctx.builder->CreateAlloca(typeDescriptionToLLVMType(ctx, typeDescription), nullptr, name);
}
