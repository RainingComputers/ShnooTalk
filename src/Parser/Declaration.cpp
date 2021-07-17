#include "Expression.hpp"

#include "Declaration.hpp"

void identifierDecleration(parser::ParserContext& ctx)
{
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::COLON);
    ctx.consume();

    typeDefinition(ctx);
}

void identifierDeclarationOptionalInit(parser::ParserContext& ctx)
{
    ctx.expect(token::IDENTIFIER);
    identifierDecleration(ctx);

    if (ctx.accept(token::EQUAL))
    {
        ctx.consume();
        expression(ctx);
    }
}

void identifierDeclareListOptionalInit(parser::ParserContext& ctx, bool initAllowed)
{
    node::NodeType declNodeType;

    ctx.expect(token::VAR);

    declNodeType = node::VAR;

    do
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(declNodeType);

        ctx.expect(token::IDENTIFIER);

        if (initAllowed)
            identifierDeclarationOptionalInit(ctx);
        else
            identifierDecleration(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));
}

void identifierDeclarationRequiredInit(parser::ParserContext& ctx)
{
    ctx.expect(token::IDENTIFIER);
    identifierDecleration(ctx);

    ctx.expect(token::EQUAL);
    ctx.consume();
    expression(ctx);
}

void identifierDeclareListRequiredInit(parser::ParserContext& ctx)
{
    node::NodeType declNodeType;

    ctx.expect(token::CONST);

    declNodeType = node::CONST;

    do
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(declNodeType);

        ctx.expect(token::IDENTIFIER);

        identifierDeclarationRequiredInit(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));
}
