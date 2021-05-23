#include "CreateDefine.hpp"

icode::Define defineFromToken(const token::Token& valueToken)
{
    if (valueToken.getType() == token::INT_LITERAL)
        return icode::createIntDefine(valueToken.toInt(), icode::INT);

    return icode::createFloatDefine(valueToken.toFloat(), icode::FLOAT);
}

void createDefine(const irgen::ir_generator& ctx, const token::Token& nameToken, const token::Token& valueToken)
{
    if (ctx.workingModule->symbolExists(nameToken.toString()))
    {
        miklog::error_tok(ctx.module.name, "Symbol already exists", ctx.file, nameToken);
        throw miklog::compile_error();
    }

    ctx.workingModule->defines[nameToken.toString()] = defineFromToken(valueToken);
}