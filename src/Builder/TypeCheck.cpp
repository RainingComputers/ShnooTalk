#include "TypeCheck.hpp"

using namespace icode;

bool dataTypeIsEqual(DataType dtype1, DataType dtype2)
{
    return dtype1 == dtype2 || (dtype1 == AUTO_INT && isInteger(dtype2)) ||
           (isInteger(dtype1) && dtype2 == AUTO_INT) || (dtype1 == AUTO_FLOAT && isFloat(dtype2)) ||
           (isFloat(dtype1) && dtype2 == AUTO_FLOAT);
}

bool isSameDim(TypeDescription type1, TypeDescription type2)
{
    if (type1.dimensions.size() != type2.dimensions.size())
        return false;

    for (size_t i = 0; i < type1.dimensions.size(); i += 1)
    {
        if (type1.dimTypes[i] == STRING_LTRL_DIM)
        {
            if (type1.dimensions[i] > type2.dimensions[i])
                return false;
        }
        else if (type2.dimTypes[i] == STRING_LTRL_DIM)
        {
            if (type2.dimensions[i] > type1.dimensions[i])
                return false;
        }
        else if (type1.dimensions[i] != type2.dimensions[i])
            return false;
    }

    return true;
}

bool isSameType(TypeDescription type1, TypeDescription type2)
{
    if (type1.isStringLtrl() && type2.isStringLtrl())
        return true;

    if (type1.dtype == STRUCT || type2.dtype == STRUCT)
        return (type1.dtypeName == type2.dtypeName && isSameDim(type1, type2) && type1.moduleName == type2.moduleName);

    return (dataTypeIsEqual(type1.dtype, type2.dtype) && isSameDim(type1, type2));
}
