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

    if (ctx.accept(token::VAR))
        declNodeType = node::VAR;
    else if (ctx.accept(token::CONST) && initAllowed)
        declNodeType = node::CONST;

    do
    {
        ctx.pushNode();

        ctx.addNode(declNodeType, true);

        ctx.expect(token::IDENTIFIER);

        if (initAllowed)
            identifierDeclarationAndInit(ctx);
        else
            identifierDecleration(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));
}
