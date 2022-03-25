#include "Literal.hpp"

#include "Expression.hpp"

void identifierWithOptionalSubscript(parser::ParserContext& ctx, bool literalSubscriptOnly)
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

void identifierWithPointerStar(parser::ParserContext& ctx)
{
    ctx.addNode(node::IDENTIFIER);
    ctx.expect(token::MULTIPLY);
    ctx.addNode(node::POINTER_STAR);
}

void identifierWithEmptySubscript(parser::ParserContext& ctx)
{
    ctx.addNode(node::IDENTIFIER);
    ctx.expect(token::EMPTY_SUBSCRIPT);
    ctx.addNode(node::EMPTY_SUBSCRIPT);
}

void identifierWithQualidentAndSubscript(parser::ParserContext& ctx)
{
    identifierWithOptionalSubscript(ctx, false);

    ctx.pushNode();

    /* The dpeek is so that it does not collide with methodCall */

    while (ctx.accept(token::DOT) && !ctx.dpeek(token::LPAREN))
    {
        ctx.addNode(node::STRUCT_FIELD);
        ctx.expect(token::IDENTIFIER);
        identifierWithOptionalSubscript(ctx, false);
    }

    ctx.popNode();
}

void genericParams(parser::ParserContext& ctx)
{

    ctx.expect(token::OPEN_SQUARE);

    do
    {
        ctx.consume();

        ctx.pushNode();

        ctx.addNodeMakeCurrentNoConsume(node::GENERIC_TYPE_PARAM);
        typeDefinition(ctx);

        ctx.popNode();

    } while (ctx.accept(token::COMMA));

    ctx.expect(token::CLOSE_SQUARE);

    ctx.consume();
}

void identifierWithGeneric(parser::ParserContext& ctx)
{
    ctx.addNode(node::IDENTIFIER);

    genericParams(ctx);
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

    if (ctx.peek(token::OPEN_SQUARE) && !ctx.dpeek(token::INT_LITERAL))
        identifierWithGeneric(ctx);
    else if (ctx.peek(token::MULTIPLY))
        identifierWithPointerStar(ctx);
    else if (ctx.peek(token::EMPTY_SUBSCRIPT))
        identifierWithEmptySubscript(ctx);
    else
        identifierWithOptionalSubscript(ctx, true);
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

void genericFunctionCall(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::GENERIC_FUNCCALL);

    genericParams(ctx);

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

    typeDefinition(ctx);

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}

void addrBuiltIn(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::ADDR);

    ctx.expect(token::LPAREN);
    ctx.consume();

    term(ctx);

    ctx.expect(token::RPAREN);
    ctx.consume();

    ctx.popNode();
}

void makeBuiltIn(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::MAKE);

    ctx.expect(token::LPAREN);
    ctx.consume();

    ctx.pushNode();
    ctx.addNodeMakeCurrentNoConsume(node::MAKE);
    typeDefinition(ctx);
    ctx.popNode();

    while (ctx.accept(token::COMMA))
    {
        ctx.consume();
        expression(ctx);
    }

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
                                    token::MINUS,        token::CONDN_NOT,   token::SIZEOF,        token::MAKE,
                                    token::ADDR };

    ctx.expect(expected, 13);

    if (ctx.accept(token::SIZEOF))
        sizeofBuiltIn(ctx);
    else if (ctx.accept(token::ADDR))
        addrBuiltIn(ctx);
    else if (ctx.accept(token::MAKE))
        makeBuiltIn(ctx);
    else if (ctx.accept(token::IDENTIFIER))
    {
        if (ctx.peek(token::LPAREN))
        {
            functionCall(ctx);
        }
        else if (ctx.matchedBracketPeek(token::OPEN_SQUARE, token::CLOSE_SQUARE, token::LPAREN))
        {
            genericFunctionCall(ctx);
        }
        else if (ctx.peek(token::CAST))
        {
            ctx.addNodeMakeCurrent(node::CAST);
            ctx.consume();
            term(ctx);
        }
        else if (ctx.peek(token::PTR_CAST))
        {
            ctx.addNodeMakeCurrent(node::PTR_CAST);
            ctx.consume();
            term(ctx);
        }
        else if (ctx.peek(token::ARRAY_PTR_CAST))
        {
            ctx.addNodeMakeCurrent(node::PTR_ARRAY_CAST);
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

void multiLineStringLiteral(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrentNoConsume(node::MULTILINE_STR_LITERAL);

    do
    {
        ctx.addNode(node::STR_LITERAL);

    } while (ctx.accept(token::STR_LITERAL));

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
                                    token::MAKE,        token::ADDR,         token::NOT };

    ctx.expect(expected, 15);

    if (ctx.accept(token::OPEN_SQUARE))
        initializerList(ctx);
    else if (ctx.accept(token::STR_LITERAL))
        multiLineStringLiteral(ctx);
    else
        baseExpression(ctx, 1);
}
