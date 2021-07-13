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

    do 
    {
        ctx.consume();
        expression(ctx);
        
    } while (ctx.accept(token::COMMA));

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}