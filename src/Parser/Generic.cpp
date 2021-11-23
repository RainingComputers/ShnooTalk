#include "Generic.hpp"

void generic(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.expect(token::GENERIC);
    ctx.addNodeMakeCurrentNoConsume(node::GENERIC);

    do
    {
        ctx.consume();
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);
    } while(ctx.accept(token::COMMA));
    
    ctx.popNode();
}