#include "CreateSymbol.hpp"
#include "Input.hpp"
#include "Print.hpp"

#include "GenerateFunction.hpp"

void generateModule(ModuleContext& ctx, BranchContext& branchContext, FormatStringsContext& formatStringsContext)
{
    setupFormatStringsContext(ctx, formatStringsContext);
    setupPrintf(ctx);
    setupScanf(ctx);

    for (auto symbol : ctx.moduleDescription.globals)
        createGlobalSymbol(ctx, symbol.second, symbol.first);

    for (auto func : ctx.moduleDescription.functions)
        generateFunction(ctx, branchContext, formatStringsContext, func.second, func.first);
}