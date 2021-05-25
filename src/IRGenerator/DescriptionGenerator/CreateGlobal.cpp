#include "CreateGlobal.hpp"

void createGlobal(irgen::ir_generator& ctx,
                  const token::Token globalNameToken,
                  const icode::VariableDescription& variableDesc)
{
    if (ctx.workingModule->symbolExists(globalNameToken.toString()))
    {
        miklog::errorOnToken(ctx.module.name, "Symbol already defined", ctx.file, globalNameToken);
        throw miklog::compile_error();
    }

    ctx.workingModule->globals[globalNameToken.toString()] = variableDesc;
}