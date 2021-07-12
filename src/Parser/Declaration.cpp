#include "Expression.hpp"

#include "Declaration.hpp"

void identifierDecleration(parser::ParserContext& ctx)
{
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::COLON);
    ctx.next();

    typeDefinition(ctx);
}

void identifierDeclarationAndInit(parser::ParserContext& ctx)
{
    ctx.expect(token::IDENTIFIER);
    identifierDecleration(ctx);

    if (ctx.accept(token::EQUAL))
    {
        ctx.next();
        expression(ctx);
    }
}

void identifierDeclareList(parser::ParserContext& ctx, bool initAllowed)
{
    node::NodeType declNodeType;

    token::TokenType expected[2] = { token::VAR, token::CONST };

    /* exclude token::CONST if initAllowed is false */
    if (initAllowed)
        ctx.expect(expected, 2);
    else
        ctx.expect(expected, 1);

    if (ctx.accept(token::CONST))
        declNodeType = node::CONST;
    else
        declNodeType = node::VAR;

    do
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(declNodeType);

        ctx.expect(token::IDENTIFIER);

        if (initAllowed)
            identifierDeclarationAndInit(ctx);
        else
            identifierDecleration(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));
}
