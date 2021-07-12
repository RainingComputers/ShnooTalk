#include "Declaration.hpp"
#include "Expression.hpp"
#include "Statement.hpp"

#include "Function.hpp"

void formalParameterList(parser::ParserContext& ctx)
{
    ctx.pushNode();

    if (ctx.accept(token::MUTABLE))
        ctx.addNode(node::MUT_PARAM, true, true);
    else
        ctx.addNode(node::PARAM, true, false);

    ctx.expect(token::IDENTIFIER);
    identifierDecleration(ctx);

    ctx.popNode();

    while (ctx.accept(token::COMMA))
    {
        ctx.next();

        ctx.pushNode();

        if (ctx.accept(token::MUTABLE))
            ctx.addNode(node::MUT_PARAM, true, true);
        else
            ctx.addNode(node::PARAM, true, false);

        ctx.expect(token::IDENTIFIER);
        identifierDecleration(ctx);

        ctx.popNode();
    }
}

void functionDefinition(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::FUNCTION, true);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::LPAREN);
    ctx.next();

    if (ctx.accept(token::IDENTIFIER) || ctx.accept(token::MUTABLE))
        formalParameterList(ctx);

    ctx.expect(token::RPAREN);
    ctx.next();

    if (ctx.accept(token::RIGHT_ARROW))
    {
        ctx.pushNode();

        ctx.next();

        ctx.expect(token::IDENTIFIER);
        typeDefinition(ctx);

        ctx.popNode();
    }

    block(ctx);

    ctx.popNode();
}
