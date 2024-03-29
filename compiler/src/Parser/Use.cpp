#include "Use.hpp"

void use(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::USE);

    ctx.expect(token::STR_LITERAL);
    ctx.addNode(node::MODULE);
    ctx.expect(token::AS);
    ctx.consume();
    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.popNode();
}

void from(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::FROM);

    token::TokenType expected[] = { token::IDENTIFIER, token::STR_LITERAL };

    ctx.expect(expected, 2);

    if (ctx.accept(token::IDENTIFIER))
        ctx.addNode(node::IDENTIFIER);
    else
        ctx.addNode(node::MODULE);

    ctx.expect(token::USE);
    ctx.addNodeMakeCurrentNoConsume(node::USE);

    do
    {
        ctx.consume();
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);

    } while (ctx.accept(token::COMMA));

    ctx.popNode();
}