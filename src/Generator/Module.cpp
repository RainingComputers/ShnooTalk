#include "Module.hpp"

int setWorkingModuleFromNode(generator::GeneratorContext& ctx, const Node& root, size_t startIndex)
{
    bool genericModuleSet = false;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::MODULE, nodeCounter); nodeCounter++)
    {
        const Token& aliasToken = root.children[nodeCounter].tok;

        if (genericModuleSet)
            ctx.console.compileErrorOnToken("Invalid MODULE ACCESS from GENERIC", aliasToken);

        if (ctx.mm.aliasExists(aliasToken))
        {
            ctx.mm.setWorkingModuleFromAlias(aliasToken);
            genericModuleSet = true;
        }
        else
        {
            ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromToken(aliasToken));
        }
    }

    return nodeCounter;
}
