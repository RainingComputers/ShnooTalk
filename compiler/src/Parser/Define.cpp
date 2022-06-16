#include "Literal.hpp"

#include "Define.hpp"

void def(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::DEF);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    if (ctx.accept(token::STR_LITERAL))
        ctx.addNode(node::STR_LITERAL);
    else
        literal(ctx);

    ctx.popNode();
}
