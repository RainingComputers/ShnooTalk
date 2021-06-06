#include "From.hpp"

void createUse(irgen::ir_generator& ctx, const Node& root)
{
    for (Node child : root.children)
    {
        const Token& nameToken = child.tok;
        ctx.descriptionFinder.createUse(child.tok);
    }
}

void createFrom(irgen::ir_generator& ctx, const Node& root)
{
    const Token& moduleNameToken = root.children[0].tok;

    for (Node child : root.children[1].children)
    {
        const Token& symbolNameToken = child.tok;
        ctx.descriptionFinder.createFrom(moduleNameToken, symbolNameToken);
    }
}