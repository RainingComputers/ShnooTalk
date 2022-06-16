#include "llvm/IR/IRBuilder.h"

#include "StackAlloca.hpp"

using namespace llvm;

Value* stackAllocName(const ModuleContext& ctx, Type* type, const std::string& name)
{
    IRBuilder<> entryBlockBuilder(&ctx.currentWorkingFunction->getEntryBlock(),
                                  ctx.currentWorkingFunction->getEntryBlock().begin());

    return entryBlockBuilder.CreateAlloca(type, nullptr, name);
}

Value* stackAlloc(const ModuleContext& ctx, Type* type)
{
    return stackAllocName(ctx, type, "");
}