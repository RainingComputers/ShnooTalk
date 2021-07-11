#include "Use.hpp"

void use(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::USE, true);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    while (ctx.accept(token::COMMA))
    {
        ctx.next();
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);
    }

    ctx.popNode();
}

void from(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::FROM, true);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::USE);
    use(ctx);

    ctx.popNode();
}