#include "Define.hpp"

void createPosDefine(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.children[0].tok;
    const Token valueToken = root.children[1].tok;

    if (valueToken.isIntLiteral())
        ctx.ir.moduleBuilder.createIntDefine(nameToken, valueToken.toInt());
    else
        ctx.ir.moduleBuilder.createFloatDefine(nameToken, valueToken.toFloat());
}

void createNegDefine(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.children[0].tok;
    const Token valueToken = root.children[2].tok;

    if (valueToken.isIntLiteral())
        ctx.ir.moduleBuilder.createIntDefine(nameToken, -1 * valueToken.toInt());
    else
        ctx.ir.moduleBuilder.createFloatDefine(nameToken, -1 * valueToken.toFloat());
}

void createStringDefine(generator::GeneratorContext& ctx, const Node& root)
{
    const Token nameToken = root.children[0].tok;
    const Token valueToken = root.children[1].tok;

    ctx.ir.moduleBuilder.createStringDefine(nameToken, valueToken);
}

void createDefineFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.getNthChildToken(1).getType() == token::MINUS)
        createNegDefine(ctx, root);
    else if (root.getNthChildToken(1).getType() == token::STR_LITERAL)
        createStringDefine(ctx, root);
    else
        createPosDefine(ctx, root);
}