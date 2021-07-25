#include "TypeCheck.hpp"

using namespace icode;

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

bool isSameType(const Unit& unit1, const Unit& unit2)
{
    if (unit1.isStringLtrl() && unit2.isStringLtrl())
        return true;

    TypeDescription type1 = unit1.type();
    TypeDescription type2 = unit2.type();

    if (type1.dtype == STRUCT || type2.dtype == STRUCT)
        return (type1.dtypeName == type2.dtypeName && isSameDim(type1, type2) && type1.moduleName == type2.moduleName);

    return (dataTypeIsEqual(type1.dtype, type2.dtype) && isSameDim(type1, type2));
}
