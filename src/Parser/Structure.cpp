#include "Declaration.hpp"

#include "Structure.hpp"

void structDefinition(parser::ParserContext& ctx)
{
    ctx.pushNode();

    ctx.addNodeMakeCurrent(node::STRUCT);

    ctx.expect(token::IDENTIFIER);
    ctx.addNodeMakeCurrent(node::IDENTIFIER);

    ctx.expect(token::OPEN_BRACE);
    ctx.next();

    while (ctx.accept(token::VAR))
        identifierDeclareList(ctx, false);

    ctx.expect(token::CLOSE_BRACE);
    ctx.next();

    ctx.popNode();
}
