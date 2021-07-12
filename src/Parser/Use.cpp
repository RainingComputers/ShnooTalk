#include "Use.hpp"

void use(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::USE);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    while (ctx.accept(token::COMMA))
    {
        ctx.consume();
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);
    }

    ctx.popNode();
}

void from(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::FROM);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::USE);
    use(ctx);

    ctx.popNode();
}