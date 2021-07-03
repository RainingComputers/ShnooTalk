#include "TypeChecker.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"
#include "TypeDescriptionUtil.hpp"

using namespace icode;

TypeChecker::TypeChecker(FunctionBuilder& functionBuilder)
  : functionBuilder(functionBuilder)
{
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

bool TypeChecker::autoCast(Unit& LHS, Unit& RHS)
{
    if (LHS.type.dtype != RHS.type.dtype)
        RHS = functionBuilder.castOperator(RHS, LHS.op.dtype);
}

bool TypeChecker::check(Unit& LHS, Unit& RHS)
{
    if (!LHS.type.isStructOrArray() && !RHS.type.isStructOrArray())
    {
        autoCast(LHS, RHS);
        return true;
    }

    return isSameType(LHS.type, RHS.type);
}
