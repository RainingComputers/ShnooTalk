#include "Use.hpp"

void createUse(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.moduleBuilder.createUse(root.getNthChildToken(0), root.getNthChildToken(1));
}

void createFrom(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& moduleNameToken = root.children[0].tok;

    for (Node child : root.children[1].children)
        ctx.ir.moduleBuilder.createFrom(moduleNameToken, child.tok);
}