#include "CreateFunctionDescription.hpp"

void createFunctionDescription(irgen::ir_generator& ctx,
                               const token::Token& nameToken,
                               const icode::VariableDescription& returnType,
                               const std::vector<token::Token>& paramNames,
                               const std::vector<icode::VariableDescription>& paramTypes)
{
    if (ctx.workingModule->symbolExists(nameToken.toString()))
        ctx.console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::FunctionDescription functionDescription;
    functionDescription.functionReturnDescription = returnType;
    functionDescription.moduleName = ctx.workingModule->name;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        if (ctx.workingModule->symbolExists(paramNames[i].toString()))
            ctx.console.compileErrorOnToken("Symbol already defined", paramNames[i]);

        /* Append to symbol table */
        functionDescription.parameters.push_back(paramNames[i].toString());
        functionDescription.symbols[paramNames[i].toString()] = paramTypes[i];
    }

    ctx.workingModule->functions[nameToken.toString()] = functionDescription;
}