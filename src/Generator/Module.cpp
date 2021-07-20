#include "Module.hpp"

int setWorkingModuleFromNode(generator::GeneratorContext& ctx, const Node& root, size_t startIndex)
{
    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::MODULE, nodeCounter); nodeCounter++)
    {
        const Token& moduleNameToken = root.children[nodeCounter].tok;
        ctx.ir.setWorkingModule(ctx.ir.descriptionFinder.getModuleFromToken(moduleNameToken));
    }

    return nodeCounter;
}
