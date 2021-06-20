#include "TypeDescriptionUtil.hpp"

using namespace icode;

TypeDescription typeDescriptionFromDataType(DataType dtype)
{
    TypeDescription var;
    var.dtype = dtype;
    var.dtypeName = dataTypeToString(dtype);
    var.dtypeSize = getDataTypeSize(dtype);
    var.size = var.dtypeSize;
    var.offset = 0;
    return var;
}

TypeDescription createArrayTypeDescription(const TypeDescription& typeDescription, std::vector<int>& dimensions)
{
    TypeDescription modifiedTypeDescription = typeDescription;

    for (int dim : dimensions)
    {
        modifiedTypeDescription.size *= dim;
        modifiedTypeDescription.dimensions.push_back(dim);
    }

    return modifiedTypeDescription;
}

TypeDescription stringTypeFromToken(const Token& token)
{
    TypeDescription stringType = typeDescriptionFromDataType(icode::UI8);

    std::vector<int> dimensions;
    /* +1 for null char */
    dimensions.push_back(token.toUnescapedString().size());

    stringType = createArrayTypeDescription(stringType, dimensions);

    stringType.becomeString();

    return stringType;
}