#include "Declaration.hpp"

#include "Structure.hpp"

void structDefinition(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNode(node::STRUCT, true);

    ctx.expect(token::IDENTIFIER);
    ctx.addNode(node::IDENTIFIER, true);

    ctx.expect(token::OPEN_BRACE);
    ctx.next();

    while (ctx.accept(token::VAR))
    {
        ctx.expect(token::VAR);
        identifierDeclareList(ctx, false);
    }

    ctx.expect(token::CLOSE_BRACE);
    ctx.next();

    ctx.popNode();
}
