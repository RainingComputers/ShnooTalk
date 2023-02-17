#include "Literal.hpp"

void literal(parser::ParserContext& ctx)
{
    token::TokenType literals[] = { token::INT_LITERAL,
                                    token::CHAR_LITERAL,
                                    token::FLOAT_LITERAL,
                                    token::HEX_LITERAL,
                                    token::BIN_LITERAL };

    if (ctx.accept(token::MINUS) || ctx.accept(token::PLUS))
    {
        ctx.addNode(node::UNARY_OPR);
        ctx.expect(literals, 5);
    }
    else
        ctx.expect(literals, 5);

    ctx.addNode(node::LITERAL);
}
