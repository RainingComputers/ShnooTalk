#include "Declaration.hpp"
#include "Expression.hpp"
#include "Input.hpp"
#include "Print.hpp"

#include "Statement.hpp"

void assignmentOrMethodCall(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.pushNode();

    ctx.addNode(node::TERM, true, false);

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
                                        token::XOR_EQUAL,      token::EQUAL };

        ctx.expect(expected, 8);
        ctx.addNode(node::ASSIGN_OPERATOR);

        expression(ctx);
    }

    ctx.popNode();
}

void moduleFunctionCall(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNode(node::MODULE, true, false);
    moduleQualident(ctx);
    functionCall(ctx);
    ctx.popNode();
}

void ifStatement(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNode(node::IF, true, false);

    ctx.pushNode();
    ctx.addNode(node::IF, true);
    expression(ctx);
    block(ctx);
    ctx.popNode();

    while (ctx.accept(token::ELSEIF))
    {
        ctx.pushNode();
        ctx.addNode(node::ELSEIF, true);
        expression(ctx);
        block(ctx);
        ctx.popNode();
    }

    if (ctx.accept(token::ELSE))
    {
        ctx.pushNode();
        ctx.addNode(node::ELSE, true);
        block(ctx);
        ctx.popNode();
    }

    ctx.popNode();
}

void whileLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();
    ctx.addNode(node::WHILE, true);
    expression(ctx);
    block(ctx);
    ctx.popNode();
}

void forLoop(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::FOR, true);

    if (ctx.accept(token::LPAREN))
        ctx.next();

    if (ctx.accept(token::VAR))
        identifierDeclareList(ctx, true);
    else
        assignmentOrMethodCall(ctx);

    ctx.expect(token::SEMICOLON);
    ctx.next();

    expression(ctx);
    ctx.expect(token::SEMICOLON);
    ctx.next();

    ctx.expect(token::IDENTIFIER);
    assignmentOrMethodCall(ctx);

    if (ctx.accept(token::RPAREN))
        ctx.next();

    block(ctx);

    ctx.popNode();
}

void returnExpression(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::RETURN, true);

    if (ctx.accept(token::VOID))
        ctx.next();
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
    else if (ctx.accept(token::VAR) || ctx.accept(token::CONST))
        identifierDeclareList(ctx, true);
    else if (ctx.accept(token::IF))
        ifStatement(ctx);
    else if (ctx.accept(token::WHILE))
        whileLoop(ctx);
    else if (ctx.accept(token::FOR))
        forLoop(ctx);
    else if (ctx.accept(token::BREAK))
        ctx.addNode(node::BREAK);
    else if (ctx.accept(token::CONTINUE))
        ctx.addNode(node::CONTINUE);
    else if (ctx.accept(token::INPUT))
        input(ctx);
    else if (ctx.accept(token::EXIT))
        ctx.addNode(node::EXIT);
    else if (ctx.accept(token::RETURN))
        returnExpression(ctx);
    else if (ctx.accept(token::PRINTLN) || ctx.accept(token::PRINT))
        print(ctx);
}

void block(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::BLOCK, true, false);

    token::TokenType expected[] = { token::CLOSE_BRACE, token::VAR,    token::CONST, token::IF,
                                    token::WHILE,       token::FOR,    token::BREAK, token::CONTINUE,
                                    token::IDENTIFIER,  token::RETURN, token::PRINT, token::PRINTLN,
                                    token::INPUT,       token::EXIT };

    if (ctx.accept(token::OPEN_BRACE))
    {
        ctx.next();

        while (!ctx.accept(token::CLOSE_BRACE))
        {
            ctx.expect(expected, 14);
            statement(ctx);
        }

        ctx.expect(token::CLOSE_BRACE);
        ctx.next();
    }
    else
    {
        ctx.expect(expected, 14);
        statement(ctx);
    }

    ctx.popNode();
}
