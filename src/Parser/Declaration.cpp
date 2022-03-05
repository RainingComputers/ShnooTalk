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

    if (ctx.accept(token::EQUAL) || ctx.accept(token::LEFT_ARROW))
    {
        ctx.addNode(node::ASSIGN_OPERATOR);
        expression(ctx);
    }
}

void identifierDeclareListOptionalInit(parser::ParserContext& ctx, bool initAllowed)
{
    do
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(node::VAR);

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

    token::TokenType expected[2] = { token::EQUAL, token::LEFT_ARROW };
    ctx.expect(expected, 2);
    ctx.addNode(node::ASSIGN_OPERATOR);

    expression(ctx);
}

void identifierDeclareListRequiredInit(parser::ParserContext& ctx)
{
    do
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(node::CONST);

        ctx.expect(token::IDENTIFIER);

        identifierDeclarationRequiredInit(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));
}

void walrusDeclaration(parser::ParserContext& ctx)
{
    ctx.pushNode();

    if (ctx.accept(token::CONST))
        ctx.addNodeMakeCurrent(node::WALRUS_CONST);
    else if (ctx.accept(token::VAR))
        ctx.addNodeMakeCurrent(node::WALRUS_VAR);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::WALRUS);
    ctx.consume();

    expression(ctx);

    ctx.popNode();
}

void destructureList(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.expect(token::OPEN_SQUARE);
    ctx.addNodeMakeCurrent(node::DESTRUCTURE_LIST);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    while (ctx.accept(token::COMMA))
    {
        ctx.consume();

        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::IDENTIFIER);
    }

    ctx.expect(token::CLOSE_SQUARE);
    ctx.consume();

    ctx.popNode();
}

void destructureDeclaration(parser::ParserContext& ctx)
{
    ctx.pushNode();

    if (ctx.accept(token::CONST))
        ctx.addNodeMakeCurrent(node::DESTRUCTURE_CONST);
    else if (ctx.accept(token::VAR))
        ctx.addNodeMakeCurrent(node::DESTRUCTURE_VAR);

    destructureList(ctx);

    ctx.expect(token::WALRUS);
    ctx.consume();

    expression(ctx);

    ctx.popNode();
}