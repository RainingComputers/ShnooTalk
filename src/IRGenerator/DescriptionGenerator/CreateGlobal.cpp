#include "CreateGlobal.hpp"

void createGlobal(irgen::ir_generator& ctx,
                  const token::Token globalNameToken,
                  const icode::VariableDescription& variableDesc)
{
    if (ctx.workingModule->symbolExists(globalNameToken.toString()))
        ctx.console.compileErrorOnToken("Symbol already defined", globalNameToken);

    ctx.workingModule->globals[globalNameToken.toString()] = variableDesc;
}