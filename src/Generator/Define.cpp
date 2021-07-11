#include "Define.hpp"

void createDefineFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.children[0].tok;
    const Token& valueToken = root.children[1].tok;

    ctx.moduleBuilder.createDefine(nameToken, valueToken);
}