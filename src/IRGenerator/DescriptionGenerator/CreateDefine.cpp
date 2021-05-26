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
        ctx.console.compileErrorOnToken("Symbol already exists", nameToken);

    ctx.workingModule->defines[nameToken.toString()] = defineFromToken(valueToken);
}