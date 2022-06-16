#include "Enum.hpp"

void enumList(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.consume();

    ctx.expect(token::IDENTIFIER);
    ctx.addNodeMakeCurrent(node::ENUM);

    ctx.expect(token::OPEN_BRACE);

    do
    {
        ctx.consume();
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);

    } while (ctx.accept(token::COMMA));

    ctx.expect(token::CLOSE_BRACE);
    ctx.consume();

    ctx.popNode();
}