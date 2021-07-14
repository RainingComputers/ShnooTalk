#include "Declaration.hpp"
#include "Expression.hpp"
#include "Statement.hpp"

#include "Function.hpp"

void formalParameterList(parser::ParserContext& ctx)
{
    do 
    {
        ctx.consume();

        ctx.pushNode();

        if (ctx.accept(token::MUTABLE))
            ctx.addNodeMakeCurrent(node::MUT_PARAM);
        else
            ctx.addNodeMakeCurrentNoConsume(node::PARAM);

        ctx.expect(token::IDENTIFIER);
        identifierDecleration(ctx);

        ctx.popNode();
    } while (ctx.accept(token::COMMA));
}

void functionDefinition(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::FUNCTION);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::LPAREN);

    if (ctx.peek(token::IDENTIFIER) || ctx.peek(token::MUTABLE))
        formalParameterList(ctx);
    else 
        ctx.consume();

    ctx.expect(token::RPAREN);
    ctx.consume();

    if (ctx.accept(token::RIGHT_ARROW))
    {
        ctx.pushNode();

        ctx.consume();

        ctx.expect(token::IDENTIFIER);
        typeDefinition(ctx);

        ctx.popNode();
    }

    block(ctx);

    ctx.popNode();
}
