#include "CreateStructDescription.hpp"

icode::StructDescription createEmptyStructDescription(const irgen::ir_generator& ctx)
{
    icode::StructDescription structDescription;
    structDescription.moduleName = ctx.workingModule->name;
    structDescription.size = 0;

    return structDescription;
}

void createStructDescription(irgen::ir_generator& ctx,
                             const token::Token& nameToken,
                             const std::vector<token::Token>& fieldNames,
                             const std::vector<icode::VariableDescription>& fieldTypes)
{
    if (ctx.workingModule->symbolExists(nameToken.toString()))
        ctx.console.compileErrorOnToken("Symbol already defined", nameToken);

    icode::StructDescription structDescription = createEmptyStructDescription(ctx);

    for (size_t i = 0; i < fieldNames.size(); i++)
    {
        if (ctx.workingModule->symbolExists(fieldNames[i].toString()))
            ctx.console.compileErrorOnToken("Symbol already defined", fieldNames[i]);

        if (structDescription.fieldExists(fieldNames[i].toString()))
            ctx.console.compileErrorOnToken("Field already defined", fieldNames[i]);

        icode::VariableDescription field = fieldTypes[i];
        field.offset = structDescription.size;

        structDescription.size += field.size;

        structDescription.structFields[fieldNames[i].toString()] = field;
    }

    ctx.workingModule->structures[nameToken.toString()] = structDescription;
}