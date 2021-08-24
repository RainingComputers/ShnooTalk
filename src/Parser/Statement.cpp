#include "Declaration.hpp"
#include "Expression.hpp"
#include "Input.hpp"
#include "Print.hpp"

#include "Statement.hpp"

void assignmentOrMethodCall(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.pushNode();

    ctx.addNodeMakeCurrentNoConsume(node::TERM);

    ctx.expect(token::IDENTIFIER);
    identifierWithQualidentAndSubscript(ctx);

    if (ctx.accept(token::DOT))
    {
        methodCall(ctx);
        ctx.popNode();
    }
    else
    {
        ctx.popNode();

        ctx.insertNode(node::ASSIGNMENT);

        token::TokenType expected[] = { token::PLUS_EQUAL,     token::MINUS_EQUAL, token::DIVIDE_EQUAL,
                                        token::MULTIPLY_EQUAL, token::OR_EQUAL,    token::AND_EQUAL,
                                        token::XOR_EQUAL,      token::EQUAL,       token::LEFT_ARROW };

        ctx.expect(expected, 9);
        ctx.addNode(node::ASSIGN_OPERATOR);

        expression(ctx);
    }

    ctx.popNode();
}

void moduleFunctionCall(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNodeMakeCurrentNoConsume(node::MODULE);
    moduleQualident(ctx);
    functionCall(ctx);
    ctx.popNode();
}

void ifStatement(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNodeMakeCurrentNoConsume(node::IF);

    ctx.pushNode();
    ctx.addNodeMakeCurrent(node::IF);
    expression(ctx);
    block(ctx);
    ctx.popNode();

    while (ctx.accept(token::ELSEIF))
    {
        ctx.pushNode();
        ctx.addNodeMakeCurrent(node::ELSEIF);
        expression(ctx);
        block(ctx);
        ctx.popNode();
    }

    if (ctx.accept(token::ELSE))
    {
        ctx.pushNode();
        ctx.addNodeMakeCurrent(node::ELSE);
        block(ctx);
        ctx.popNode();
    }

    ctx.popNode();
}

void whileLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNodeMakeCurrent(node::WHILE);
    expression(ctx);
    block(ctx);
    ctx.popNode();
}

void doWhileLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::DO_WHILE);

    block(ctx);

    ctx.expect(token::WHILE);
    ctx.consume();

    expression(ctx);

    ctx.popNode();
}

void forLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::FOR);

    if (ctx.accept(token::LPAREN))
        ctx.consume();

    if (ctx.accept(token::VAR))
        identifierDeclareListOptionalInit(ctx, true);
    else
        assignmentOrMethodCall(ctx);

    ctx.expect(token::SEMICOLON);
    ctx.consume();

    expression(ctx);

    ctx.expect(token::SEMICOLON);
    ctx.consume();

    ctx.expect(token::IDENTIFIER);
    assignmentOrMethodCall(ctx);

    if (ctx.accept(token::RPAREN))
        ctx.consume();

    block(ctx);

    ctx.popNode();
}

void infniteLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::LOOP);

    block(ctx);

    ctx.popNode();
}

void returnExpression(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::RETURN);

    if (ctx.accept(token::VOID))
        ctx.consume();
    else
        expression(ctx);

    ctx.popNode();
}

void statement(parser::ParserContext& ctx)
{
    if (ctx.accept(token::IDENTIFIER))
    {
        if (ctx.peek(token::LPAREN))
            functionCall(ctx);
        else if (ctx.peek(token::DOUBLE_COLON))
            moduleFunctionCall(ctx);
        else
            assignmentOrMethodCall(ctx);
    }
    else if (ctx.accept(token::VAR))
        identifierDeclareListOptionalInit(ctx, true);
    else if (ctx.accept(token::CONST))
        identifierDeclareListRequiredInit(ctx);
    else if (ctx.accept(token::IF))
        ifStatement(ctx);
    else if (ctx.accept(token::WHILE))
        whileLoop(ctx);
    else if (ctx.accept(token::DO))
        doWhileLoop(ctx);
    else if (ctx.accept(token::FOR))
        forLoop(ctx);
    else if (ctx.accept(token::LOOP))
        infniteLoop(ctx);
    else if (ctx.accept(token::BREAK))
        ctx.addNode(node::BREAK);
    else if (ctx.accept(token::CONTINUE))
        ctx.addNode(node::CONTINUE);
    else if (ctx.accept(token::PRINTLN) || ctx.accept(token::PRINT))
        print(ctx);
    else if (ctx.accept(token::INPUT))
        input(ctx);
    else if (ctx.accept(token::RETURN))
        returnExpression(ctx);
}

void block(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrentNoConsume(node::BLOCK);

    token::TokenType expected[] = {
        token::CLOSE_BRACE, token::VAR,    token::CONST, token::IF,      token::WHILE,
        token::DO,          token::FOR,    token::LOOP,  token::BREAK,   token::CONTINUE,
        token::IDENTIFIER,  token::RETURN, token::PRINT, token::PRINTLN, token::INPUT,
    };

    if (ctx.accept(token::OPEN_BRACE))
    {
        ctx.consume();

        while (!ctx.accept(token::CLOSE_BRACE))
        {
            ctx.expect(expected, 15);
            statement(ctx);
        }

        ctx.expect(token::CLOSE_BRACE);
        ctx.consume();
    }
    else
    {
        ctx.expect(expected, 15);
        statement(ctx);
    }

    ctx.popNode();
}
