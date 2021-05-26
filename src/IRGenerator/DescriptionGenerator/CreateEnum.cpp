#include "CreateEnum.hpp"

void createEnum(irgen::ir_generator& ctx, const std::vector<token::Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (ctx.workingModule->symbolExists(enums[i].toString()))
            ctx.console.compileErrorOnToken("Symbol already defined", enums[i]);

        ctx.workingModule->enumerations[enums[i].toString()] = i;
    }
}