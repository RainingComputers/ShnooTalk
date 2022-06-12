#ifndef BUILDER_TYPE_DESCRIPTION_UTIL
#define BUILDER_TYPE_DESCRIPTION_UTIL

#include "../IntermediateRepresentation/DataType.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"
#include "../Token/Token.hpp"

icode::TypeDescription typeDescriptionFromDataType(icode::DataType dtype);

icode::TypeDescription createArrayTypeDescription(const icode::TypeDescription& typeDescription,
                                                  std::vector<int>& dimensions,
                                                  icode::DimensionType dimType);

icode::TypeDescription prependDimension(const icode::TypeDescription& typeDescription,
                                        int dimension,
                                        icode::DimensionType dimType);

icode::TypeDescription getElementType(const icode::TypeDescription& typeDescription);

#endif
