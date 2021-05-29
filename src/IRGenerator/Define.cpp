#include "Define.hpp"

void createDefineFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    const token::Token& nameToken = root.children[0].tok;
    const token::Token& valueToken = root.children[1].tok;

    ctx.descriptionBuilder.createDefine(nameToken, valueToken);

    ctx.scope.putInGlobalScope(nameToken);
}