#include "Declaration.hpp"
#include "Define.hpp"
#include "Enum.hpp"
#include "Function.hpp"
#include "Structure.hpp"
#include "Use.hpp"

#include "Module.hpp"

void programModule(parser::ParserContext& ctx)
{
    token::TokenType expected[] = { token::STRUCT, token::FUNCTION, token::ENUM,
                                    token::DEF,    token::VAR,      token::END_OF_FILE };

    while (ctx.accept(token::USE))
        use(ctx);

    while (ctx.accept(token::FROM))
        from(ctx);

    while (true)
    {
        ctx.expect(expected, 6);

        if (ctx.accept(token::DEF))
            def(ctx);
        else if (ctx.accept(token::ENUM))
            enumList(ctx);
        else if (ctx.accept(token::VAR))
            identifierDeclareListOptionalInit(ctx, false);
        else if (ctx.accept(token::STRUCT))
            structDefinition(ctx);
        else if (ctx.accept(token::FUNCTION))
            functionDefinition(ctx);
        else
            break;
    }

    ctx.expect(token::END_OF_FILE);
}