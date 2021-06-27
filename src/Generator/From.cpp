#include "From.hpp"

void createUse(irgen::ir_generator& ctx, const Node& root)
{
    for (Node child : root.children)
        ctx.moduleBuilder.createUse(child.tok);
}

void createFrom(irgen::ir_generator& ctx, const Node& root)
{
    const Token& moduleNameToken = root.children[0].tok;

    for (Node child : root.children[1].children)
        ctx.moduleBuilder.createFrom(moduleNameToken, child.tok);
}