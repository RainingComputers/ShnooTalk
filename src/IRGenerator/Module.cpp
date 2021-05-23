#include "Module.hpp"

ModuleIndexPair getModuleFromNode(const irgen::ir_generator& ctx, const node::Node& root, size_t startIndex)
{
    size_t nodeCounter;
    icode::ModuleDescription* currentModule = &ctx.module;

    for (nodeCounter = startIndex; root.isNthChild(node::MODULE, nodeCounter); nodeCounter++)
    {
        const token::Token& moduleNameToken = root.children[nodeCounter].tok;
        const std::string& moduleName = moduleNameToken.toString();

        if (!(*currentModule).useExists(moduleName))
        {
            miklog::error_tok(ctx.module.name, "Module does not exist", ctx.file, moduleNameToken);
            throw miklog::compile_error();
        }

        currentModule = &ctx.ext_modules_map[moduleName];
    }

    return ModuleIndexPair(currentModule, nodeCounter);
}
