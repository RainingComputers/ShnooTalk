#include "Expression.hpp"

#include "Input.hpp"

void input(parser::ParserContext& ctx)
{
    ctx.addNodeMakeCurrent(node::INPUT);

    ctx.expect(token::LPAREN);
    ctx.consume();

    ctx.expect(token::IDENTIFIER);
    term(ctx);

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}