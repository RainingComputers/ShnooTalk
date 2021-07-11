#include "Expression.hpp"

#include "Print.hpp"

void print(parser::ParserContext& ctx)
{
    ctx.pushNode();

    if (ctx.accept(token::PRINTLN))
        ctx.addNode(node::PRINTLN, true);
    else
        ctx.addNode(node::PRINT, true);

    ctx.expect(token::LPAREN);
    ctx.next();

    expression(ctx);

    while (ctx.accept(token::COMMA))
    {
        ctx.next();
        expression(ctx);
    }

    ctx.expect(token::RPAREN);
    ctx.next();

    ctx.popNode();
}