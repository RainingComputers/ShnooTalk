#include "Define.hpp"

void createDefineFromNode(irgen::ir_generator& ctx, const Node& root)
{
    const Token& nameToken = root.children[0].tok;
    const Token& valueToken = root.children[1].tok;

    ctx.descriptionBuilder.createDefine(nameToken, valueToken);
}