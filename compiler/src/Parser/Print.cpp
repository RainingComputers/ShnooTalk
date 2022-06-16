#include "Expression.hpp"

#include "Print.hpp"

void print(parser::ParserContext& ctx)
{
    ctx.pushNode();

    if (ctx.accept(token::PRINTLN))
        ctx.addNodeMakeCurrent(node::PRINTLN);
    else
        ctx.addNodeMakeCurrent(node::PRINT);

    ctx.expect(token::LPAREN);
    ctx.consume();

    while (!ctx.accept(token::RPAREN))
    {
        if (ctx.accept(token::COMMA))
            ctx.addNode(node::SPACE);
        else if (ctx.accept(token::SEMICOLON))
            ctx.consume();
        else
            expression(ctx);
    };

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}