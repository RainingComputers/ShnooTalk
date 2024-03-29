#include "CreateSymbol.hpp"
#include "Print.hpp"

#include "GenerateFunction.hpp"

void generateModule(ModuleContext& ctx, BranchContext& branchContext, FormatStringsContext& formatStringsContext)
{
    setupFormatStringsContext(ctx, formatStringsContext);
    setupPrintf(ctx);

    for (auto symbol : ctx.moduleDescription.globals)
        createGlobalSymbol(ctx, symbol.second, symbol.first);

    for (auto globalString : ctx.moduleDescription.stringsData)
        createGlobalString(ctx, globalString.first, globalString.second);

    for (auto func : ctx.moduleDescription.functions)
        generateFunction(ctx, branchContext, formatStringsContext, func.second, func.first);
}