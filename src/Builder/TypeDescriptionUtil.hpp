#ifndef BUILDER_TYPE_DESCRIPTION_UTIL
#define BUILDER_TYPE_DESCRIPTION_UTIL

#include "../IntermediateRepresentation/DataType.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"
#include "../Token/Token.hpp"

icode::TypeDescription typeDescriptionFromDataType(icode::DataType dtype);

icode::TypeDescription createArrayTypeDescription(const icode::TypeDescription& typeDescription,
                                                  std::vector<int>& dimensions);

icode::TypeDescription prependDimension(const icode::TypeDescription& typeDescription, int dimension);

#endif
