#include "Define.hpp"

void def(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::DEF, true);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER);

    token::TokenType expected[] = { token::INT_LITERAL,
                                    token::FLOAT_LITERAL,
                                    token::CHAR_LITERAL,
                                    token::BIN_LITERAL,
                                    token::HEX_LITERAL };

    ctx.expect(expected, 5);

    ctx.addNode(node::LITERAL);

    ctx.popNode();
}
