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

TypeDescription createArrayTypeDescription(const TypeDescription& typeDescription, std::vector<int>& dimensions, DimensionType dimType)
{
    TypeDescription modifiedTypeDescription = typeDescription;

    for (int dim : dimensions)
    {
        modifiedTypeDescription.size *= dim;
        modifiedTypeDescription.dimensions.push_back(dim);
        modifiedTypeDescription.dimTypes.push_back(dimType);
    }

    return modifiedTypeDescription;
}

TypeDescription prependDimension(const TypeDescription& typeDescription, int dimension, DimensionType dimType)
{
    TypeDescription modifiedTypeDescription = typeDescription;

    modifiedTypeDescription.size *= dimension;
    modifiedTypeDescription.dimensions.insert(modifiedTypeDescription.dimensions.begin(), dimension);
    modifiedTypeDescription.dimTypes.insert(modifiedTypeDescription.dimTypes.begin(), dimType);

    return modifiedTypeDescription;
}
