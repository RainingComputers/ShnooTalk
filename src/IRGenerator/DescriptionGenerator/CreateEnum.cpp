#include "CreateEnum.hpp"

void createEnum(irgen::ir_generator& ctx, const std::vector<token::Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (ctx.module.symbolExists(enums[i].string))
        {
            miklog::error_tok(ctx.module.name, "Symbol already defined", ctx.file, enums[i]);
            throw miklog::compile_error();
        }

        ctx.module.enumerations[enums[i].string] = i;
    }
}