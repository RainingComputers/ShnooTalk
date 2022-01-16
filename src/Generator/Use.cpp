#include "Use.hpp"

void generateModule(generator::GeneratorContext& ctx, const Node& root);

namespace generator
{
    Node generateAST(Console& console);
}

bool generateIRFromName(generator::GeneratorContext& ctx, const std::string& moduleName)
{
    if (ctx.moduleExists(moduleName))
        return false;

    if (ctx.genericModuleExists(moduleName))
        return true;

    ctx.console.pushModule(moduleName);

    Node ast = generator::generateAST(ctx.console);

    const bool isGeneric = ast.isGenericModule();

    if (isGeneric)
        ctx.mm.indexAST(moduleName, ast);
    else
    {
        generator::GeneratorContext generatorContext = ctx.clone(moduleName);
        generateModule(generatorContext, ast);
    }

    ctx.console.popModule();

    return isGeneric;
}

void createUse(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& moduleNameToken = root.getNthChildToken(0);
    const Token& aliastoken = root.getNthChildToken(1);

    ctx.ir.moduleBuilder.createUse(moduleNameToken, aliastoken);

    bool isGeneric = generateIRFromName(ctx, moduleNameToken.toUnescapedString());

    if (isGeneric)
        ctx.mm.createUse(moduleNameToken, aliastoken);
}

void createFrom(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& aliasToken = root.children[0].tok;

    if (ctx.mm.aliasExists(aliasToken))
    {
        for (const Node& child : root.children[1].children)
            ctx.mm.createFrom(aliasToken, child.tok);
    }
    else
    {
        for (const Node& child : root.children[1].children)
            ctx.ir.moduleBuilder.createFrom(aliasToken, child.tok);
    }
}