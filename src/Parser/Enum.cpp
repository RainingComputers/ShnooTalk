#include "Enum.hpp"

void enumList(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::ENUM, true);

    ctx.expect(token::OPEN_SQUARE);
    ctx.next();

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    while (ctx.accept(token::COMMA))
    {
        ctx.next();

        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);
    }

    ctx.expect(token::CLOSE_SQUARE);
    ctx.next();

    ctx.popNode();
}