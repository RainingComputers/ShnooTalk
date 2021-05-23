#include "CreateDefine.hpp"

icode::Define defineFromToken(const token::Token& valueToken)
{
    if (valueToken.type == token::INT_LITERAL)
        return icode::createIntDefine(valueToken.toInt(), icode::INT);

    return icode::createFloatDefine(valueToken.toFloat(), icode::FLOAT);
}

void createDefine(const irgen::ir_generator& ctx, const token::Token& nameToken, const token::Token& valueToken) 
{
    if (ctx.module.symbolExists(nameToken.string))
    {
        miklog::error_tok(ctx.module.name, "Symbol already exists", ctx.file, nameToken);
        throw miklog::compile_error();
    }

    ctx.module.defines[nameToken.string] = defineFromToken(valueToken);
}