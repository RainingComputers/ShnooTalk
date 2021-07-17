#include "Literal.hpp"

#include "Define.hpp"

void def(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::DEF);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    literal(ctx);

    ctx.popNode();
}
