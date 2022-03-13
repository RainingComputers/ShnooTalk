#include "Use.hpp"

void generateModule(generator::GeneratorContext& ctx, const Node& root);

namespace generator
{
    Node generateAST(Console& console);
}

bool generateIROrMonomorphizedASTFromName(generator::GeneratorContext& ctx, const std::string& moduleName)
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
    const Token& pathToken = root.getNthChildToken(0);
    const Token& aliastoken = root.getNthChildToken(1);

    ctx.ir.moduleBuilder.createUse(pathToken, aliastoken);

    bool isGeneric = generateIROrMonomorphizedASTFromName(ctx, pathToken.toUnescapedString());

    if (isGeneric)
        ctx.mm.createUse(pathToken, aliastoken);
}

void createAliasFrom(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& aliasToken = root.children[0].tok;

    if (ctx.mm.aliasExists(aliasToken))
    {
        for (const Node& child : root.children[1].children)
            ctx.mm.createAliasFrom(aliasToken, child.tok);
    }
    else
    {
        for (const Node& child : root.children[1].children)
            ctx.ir.moduleBuilder.createAliasFrom(aliasToken, child.tok);
    }
}

void createDirectFrom(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& pathToken = root.children[0].tok;

    ctx.ir.moduleBuilder.createUseNoAlias(pathToken);

    bool isGeneric = generateIROrMonomorphizedASTFromName(ctx, pathToken.toUnescapedString());

    if (isGeneric)
    {
        for (const Node& child : root.children[1].children)
            ctx.mm.createDirectFrom(pathToken, child.tok);
    }
    else
    {
        for (const Node& child : root.children[1].children)
            ctx.ir.moduleBuilder.createDirectFrom(pathToken, child.tok);
    }
}

void createFrom(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.children[0].tok.getType() == token::STR_LITERAL)
        createDirectFrom(ctx, root);
    else
        createAliasFrom(ctx, root);
}
