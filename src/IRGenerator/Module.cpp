#include "Module.hpp"

int setWorkingModuleFromNode(irgen::ir_generator& ctx, const Node& root, size_t startIndex)
{
    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::MODULE, nodeCounter); nodeCounter++)
    {
        const Token& moduleNameToken = root.children[nodeCounter].tok;
        const std::string& moduleName = moduleNameToken.toString();

        ctx.setWorkingModule(ctx.descriptionFinder.getModuleFromToken(moduleNameToken, ctx.ext_modules_map));
    }

    return nodeCounter;
}
