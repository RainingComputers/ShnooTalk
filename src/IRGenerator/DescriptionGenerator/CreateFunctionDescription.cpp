#include "CreateFunctionDescription.hpp"

void createFunctionDescription(irgen::ir_generator& ctx,
                               const token::Token& nameToken,
                               const icode::VariableDescription& returnType,
                               const std::vector<token::Token>& paramNames,
                               const std::vector<icode::VariableDescription>& paramTypes)
{
    if (ctx.workingModule->symbolExists(nameToken.toString()))
    {
        miklog::errorOnToken(ctx.module.name, "Symbol already defined", ctx.file, nameToken);
        throw miklog::compile_error();
    }

    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnDescription = returnType;
    functionDescription.moduleName = ctx.workingModule->name;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        if (ctx.workingModule->symbolExists(paramNames[i].toString()))
        {
            miklog::errorOnToken(ctx.module.name, "Symbol already defined", ctx.file, paramNames[i]);
            throw miklog::compile_error();
        }

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    ctx.workingModule->functions[nameToken.toString()] = functionDescription;
}