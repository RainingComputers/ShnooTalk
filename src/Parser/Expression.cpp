#include "Literal.hpp"

#include "Expression.hpp"

void identifierWithSubscript(parser::ParserContext& ctx, bool literalSubscriptOnly)
{
    ctx.addNode(node::IDENTIFIER);

    while (ctx.accept(token::OPEN_SQUARE))
    {
        ctx.pushNode();

        ctx.addNodeMakeCurrent(node::SUBSCRIPT);

        if (literalSubscriptOnly)
        {
            ctx.expect(token::INT_LITERAL);
            ctx.addNode(node::LITERAL);
        }
        else
            expression(ctx);

        ctx.expect(token::CLOSE_SQUARE);
        ctx.consume();

        ctx.popNode();
    }
}

void identifierWithQualidentAndSubscript(parser::ParserContext& ctx)
{
    identifierWithSubscript(ctx, false);

    ctx.pushNode();

    /* The dpeek is so that it does not collide with methodCall */

    while (ctx.accept(token::DOT) && !ctx.dpeek(token::LPAREN))
    {
        ctx.addNode(node::STRUCT_FIELD);
        ctx.expect(token::IDENTIFIER);
        identifierWithSubscript(ctx, false);
    }

    ctx.popNode();
}

void moduleQualident(parser::ParserContext& ctx)
{
    while (ctx.peek(token::DOUBLE_COLON))
    {
        ctx.expect(token::IDENTIFIER);
        ctx.addNode(node::MODULE);
        ctx.consume();
    }
}

void typeDefinition(parser::ParserContext& ctx)
{
    moduleQualident(ctx);

    ctx.expect(token::IDENTIFIER);
    identifierWithSubscript(ctx, true);
}

void actualParameterList(parser::ParserContext& ctx)
{
    ctx.expect(token::LPAREN);
    ctx.consume();

    if (!ctx.accept(token::RPAREN))
    {
        expression(ctx);

        while (ctx.accept(token::COMMA))
        {
            ctx.consume();
            expression(ctx);
        }
    }

    ctx.expect(token::RPAREN);
    ctx.consume();
}

void functionCall(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::FUNCCALL);

    actualParameterList(ctx);

    ctx.popNode();
}

void methodCall(parser::ParserContext& ctx)
{
    while (ctx.accept(token::DOT) && ctx.dpeek(token::LPAREN))
    {
        ctx.consume();

        ctx.expect(token::IDENTIFIER);

        ctx.duplicateNode();

        ctx.pushNode();

        ctx.insertNode(node::METHODCALL);
        actualParameterList(ctx);

        ctx.popNode();
    }
}

void sizeofBuiltIn(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::SIZEOF);

    ctx.expect(token::LPAREN);
    ctx.consume();

    moduleQualident(ctx);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}

void initializerList(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.expect(token::OPEN_SQUARE);
    ctx.addNodeMakeCurrentNoConsume(node::INITLIST);

    do
    {
        ctx.consume();
        expression(ctx);
        
    } while (ctx.accept(token::COMMA));

    ctx.expect(token::CLOSE_SQUARE);
    ctx.consume();

    ctx.popNode();
}

void term(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrentNoConsume(node::TERM);

    token::TokenType expected[] = { token::IDENTIFIER,   token::NOT,         token::LPAREN,        token::INT_LITERAL,
                                    token::CHAR_LITERAL, token::HEX_LITERAL, token::FLOAT_LITERAL, token::BIN_LITERAL,
                                    token::MINUS,        token::CONDN_NOT,   token::SIZEOF };

    ctx.expect(expected, 11);

    if (ctx.accept(token::SIZEOF))
        sizeofBuiltIn(ctx);
    else if (ctx.accept(token::IDENTIFIER))
    {
        if (ctx.peek(token::LPAREN))
        {
            functionCall(ctx);
        }
        else if (ctx.peek(token::CAST))
        {
            ctx.addNodeMakeCurrent(node::CAST);
            ctx.consume();
            term(ctx);
        }
        else if (ctx.peek(token::DOUBLE_COLON))
        {
            moduleQualident(ctx);
            term(ctx);
        }
        else
        {
            identifierWithQualidentAndSubscript(ctx);
            methodCall(ctx);
        }
    }
    else if (ctx.accept(token::LPAREN))
    {
        ctx.consume();
        expression(ctx);
        ctx.expect(token::RPAREN);
        ctx.consume();
    }
    else if (ctx.accept(token::NOT) || ctx.accept(token::MINUS) || ctx.accept(token::CONDN_NOT))
    {
        ctx.addNodeMakeCurrent(node::UNARY_OPR);
        term(ctx);
    }
    else
    {
        literal(ctx);
    }

    ctx.popNode();
}

void baseExpression(parser::ParserContext& ctx, int minPrecedence)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrentNoConsume(node::EXPRESSION);

    term(ctx);

    while (ctx.getOperatorPrecedence() >= minPrecedence)
    {
        int prec = ctx.getOperatorPrecedence();

        ctx.insertNodeBeginning(node::EXPRESSION);

        ctx.addNode(node::OPERATOR);

        baseExpression(ctx, prec + 1);
    }

    ctx.popNode();
}

void expression(parser::ParserContext& ctx)
{
    token::TokenType expected[] = { token::INT_LITERAL, token::CHAR_LITERAL, token::HEX_LITERAL, token::FLOAT_LITERAL,
                                    token::STR_LITERAL, token::BIN_LITERAL,  token::OPEN_SQUARE, token::IDENTIFIER,
                                    token::MINUS,       token::LPAREN,       token::CONDN_NOT,   token::SIZEOF,
                                    token::NOT };

    ctx.expect(expected, 13);

    if (ctx.accept(token::OPEN_SQUARE))
        initializerList(ctx);
    else if (ctx.accept(token::STR_LITERAL))
        ctx.addNode(node::STR_LITERAL);
    else
        baseExpression(ctx, 1);
}
