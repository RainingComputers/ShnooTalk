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
    if(ctx.workingModule->symbolExists(nameToken.toString()))
    {
        miklog::error_tok(ctx.module.name, "Symbol already defined", ctx.file, nameToken);
        throw miklog::compile_error();
    }    

    icode::StructDescription structDescription = createEmptyStructDescription(ctx);

    for (size_t i = 0; i < fieldNames.size(); i++)
    {
        if(ctx.workingModule->symbolExists(fieldNames[i].toString()))
        {
            miklog::error_tok(ctx.module.name, "Symbol already defined", ctx.file, fieldNames[i]);
            throw miklog::compile_error();
        }    

        if(structDescription.fieldExists(fieldNames[i].toString()))
        {
            miklog::error_tok(ctx.module.name, "Field already defined", ctx.file, fieldNames[i]);
            throw miklog::compile_error();            
        }

        icode::VariableDescription field = fieldTypes[i];
        field.offset = structDescription.size;
        field.setProperty(icode::IS_MUT);

        structDescription.size += field.size;

        structDescription.structFields[fieldNames[i].toString()] = field;
    }

    ctx.workingModule->structures[nameToken.toString()] = structDescription;
}