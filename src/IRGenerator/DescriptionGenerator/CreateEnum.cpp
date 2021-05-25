#include "CreateEnum.hpp"

void createEnum(irgen::ir_generator& ctx, const std::vector<token::Token>& enums)
{
    for (size_t i = 0; i < enums.size(); i += 1)
    {
        if (ctx.workingModule->symbolExists(enums[i].toString()))
        {
            miklog::errorOnToken(ctx.module.name, "Symbol already defined", ctx.file, enums[i]);
            throw miklog::compile_error();
        }

        ctx.workingModule->enumerations[enums[i].toString()] = i;
    }
}