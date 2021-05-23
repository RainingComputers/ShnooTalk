#include "CreateVariableDescription.hpp"

using namespace icode;

VariableDescription createVoidVariableDescription(const irgen::ir_generator& ctx)
{
    VariableDescription voidVariableDescription;

    voidVariableDescription.dtype = icode::VOID;
    voidVariableDescription.dtypeName = dataTypeToString(icode::VOID);
    voidVariableDescription.dtypeSize = 0;
    voidVariableDescription.size = 0;
    voidVariableDescription.offset = 0;
    voidVariableDescription.scopeId = 0;
    voidVariableDescription.properties = 0;
    voidVariableDescription.moduleName = ctx.workingModule->name;

    return voidVariableDescription;
}

std::pair<int, std::string> getSizeAndModuleName(irgen::ir_generator& ctx,
                                                 const token::Token& dataTypeToken,
                                                 DataType dtype)
{
    if (dtype != icode::STRUCT)
        return std::pair<int, std::string>(getDataTypeSize(dtype), ctx.workingModule->name);

    icode::StructDescription structDesc;
    if (!ctx.workingModule->getStruct(dataTypeToken.toString(), structDesc))
    {
        miklog::error_tok(ctx.module.name, "Symbol does not exist", ctx.file, dataTypeToken);
        throw miklog::compile_error();
    }

    return std::pair<int, std::string>(structDesc.size, structDesc.moduleName);
}

VariableDescription createVariableDescription(irgen::ir_generator& ctx, const token::Token& dataTypeToken)
{
    icode::DataType dtype = ctx.module.dataTypeFromString(dataTypeToken.toString());

    std::pair<int, std::string> sizeAndModuleName = getSizeAndModuleName(ctx, dataTypeToken, dtype);

    VariableDescription variableDescription;

    variableDescription.dtype = dtype;
    variableDescription.dtypeName = dataTypeToken.toString();
    variableDescription.dtypeSize = sizeAndModuleName.first;
    variableDescription.size = variableDescription.dtypeSize;
    variableDescription.offset = 0;
    variableDescription.scopeId = ctx.get_scope_id();
    variableDescription.properties = 0;
    variableDescription.moduleName = sizeAndModuleName.second;

    return variableDescription;
}

VariableDescription createArrayVariableDescription(const VariableDescription& variableDesc,
                                                   std::vector<int>& dimensions)
{
    VariableDescription modifiedVariableDesc = variableDesc;

    for (int dim : dimensions)
    {
        modifiedVariableDesc.size *= dim;
        modifiedVariableDesc.dimensions.push_back(dim);
    }

    return modifiedVariableDesc;
}