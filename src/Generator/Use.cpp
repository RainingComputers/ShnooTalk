#include <filesystem>

#include "Use.hpp"

void generateModule(generator::GeneratorContext& ctx, const Node& root);

namespace generator
{
    Node generateAST(Console& console);
}

bool invalidModuleName(const std::string& path)
{
    for (const char c : path)
        if (std::string("[]@:~*").find(c) != std::string::npos)
            return true;

    return false;
}

bool generateIROrMonomorphizedASTFromName(generator::GeneratorContext& ctx, const Token& pathToken)
{
    const std::string& path = pathToken.toUnescapedString();

    if (ctx.moduleExists(path))
        return false;

    if (ctx.genericModuleExists(path))
        return true;

    if (!std::filesystem::exists(path))
        ctx.console.compileErrorOnToken("File does not exist", pathToken);

    if (invalidModuleName(path))
        ctx.console.compileErrorOnToken("Invalid module name", pathToken);

    ctx.console.pushModule(path);

    Node ast = generator::generateAST(ctx.console);

    const bool isGeneric = ast.isGenericModule();

    if (isGeneric)
        ctx.mm.indexAST(path, ast);
    else
    {
        generator::GeneratorContext generatorContext = ctx.clone(path);
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

    bool isGeneric = generateIROrMonomorphizedASTFromName(ctx, pathToken);

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

    bool isGeneric = generateIROrMonomorphizedASTFromName(ctx, pathToken);

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
