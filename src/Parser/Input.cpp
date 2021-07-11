#include "Expression.hpp"

#include "Input.hpp"

void input(parser::ParserContext& ctx)
{

    ctx.addNode(node::INPUT, true);

    ctx.expect(token::LPAREN);
    ctx.next();

    ctx.expect(token::IDENTIFIER);
    term(ctx);

    ctx.expect(token::RPAREN);
    ctx.next();

    ctx.popNode();
}