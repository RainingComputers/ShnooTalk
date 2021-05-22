#include "CreateVariableDescription.hpp"

using namespace icode;

VariableDescription constructVoidVariableDesc(const irgen::ir_generator& ctx)
{
    VariableDescription voidVariableDescription;

    voidVariableDescription.dtype = icode::VOID;
    voidVariableDescription.dtypeName = dataTypeToString(icode::VOID);
    voidVariableDescription.dtypeSize = 0;
    voidVariableDescription.size = 0;
    voidVariableDescription.offset = 0;
    voidVariableDescription.scopeId = 0;
    voidVariableDescription.properties = 0;
    voidVariableDescription.moduleName = ctx.current_ext_module->name;

    return voidVariableDescription;
}

VariableDescription variableDescFromStructDataTypeToken(irgen::ir_generator& ctx, const token::Token& dataTypeToken) 
{
    icode::StructDescription structDesc;
    if(!ctx.current_ext_module->getStruct(dataTypeToken.string, structDesc))
    {
        miklog::error_tok(ctx.module.name, "Symbol does not exist", ctx.file, dataTypeToken);
        throw miklog::compile_error();
    }

    VariableDescription variableDescription;

    variableDescription.dtype = icode::STRUCT;
    variableDescription.dtypeName = dataTypeToken.string;
    variableDescription.dtypeSize = structDesc.size;
    variableDescription.size = variableDescription.dtypeSize;
    variableDescription.offset = 0;
    variableDescription.scopeId = ctx.get_scope_id();
    variableDescription.properties = 0;
    variableDescription.moduleName = structDesc.moduleName;

    return variableDescription;

}

VariableDescription variableDescFromDataTypeToken(irgen::ir_generator& ctx, const token::Token& dataTypeToken)
{
    icode::DataType dtype = dataTypeFromString(dataTypeToken.string, ctx.target);

    if (dtype == icode::STRUCT)
        return variableDescFromStructDataTypeToken(ctx, dataTypeToken);

    VariableDescription variableDescription;

    variableDescription.dtype = dtype;
    variableDescription.dtypeName = dataTypeToken.string;
    variableDescription.dtypeSize = getDataTypeSize(dtype);
    variableDescription.size = variableDescription.dtypeSize;
    variableDescription.offset = 0;
    variableDescription.scopeId = ctx.get_scope_id();
    variableDescription.properties = 0;
    variableDescription.moduleName = ctx.current_ext_module->name;

    return variableDescription;
}

VariableDescription addDimensionToVariableDesc(const VariableDescription& variableDesc, std::vector<int>& dimensions)
{
    VariableDescription modifiedVariableDesc = variableDesc;

    for (int dim : dimensions)
    {
        modifiedVariableDesc.size *= dim;
        modifiedVariableDesc.dimensions.push_back(dim);
    }

    return modifiedVariableDesc;
}