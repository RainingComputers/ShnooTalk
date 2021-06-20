#include "TypeDescription.hpp"

namespace icode
{
    TypeDescription::TypeDescription()
    {
        properties = 0;
    }

    void TypeDescription::setProperty(TypeProperties property)
    {
        properties |= (1 << property);
    }

    void TypeDescription::clearProperty(TypeProperties property)
    {
        properties &= ~(1 << property);
    }

    bool TypeDescription::checkProperty(TypeProperties property) const
    {
        return properties & (1 << property);
    }

    void TypeDescription::becomeMutable()
    {
        setProperty(IS_MUT);
    }

    void TypeDescription::becomePointer()
    {
        setProperty(IS_PTR);
    }

    void TypeDescription::becomeString()
    {
        setProperty(IS_STRING_LTRL);
    }

    bool TypeDescription::isMutable() const
    {
        return checkProperty(IS_MUT);
    }

    bool TypeDescription::isPointer() const
    {
        return checkProperty(IS_PTR);
    }

    bool TypeDescription::isStruct() const
    {
        return dtype == STRUCT;
    }

    bool TypeDescription::isArray() const
    {
        return dimensions.size() > 0;
    }

    bool TypeDescription::isIntegerType() const
    {
        return isInteger(dtype);
    }

    bool TypeDescription::isString() const
    {
        return checkProperty(IS_STRING_LTRL);
    }

    bool canAssignString(TypeDescription assignee, TypeDescription str)
    {
        if (assignee.dimensions.size() != 1 || assignee.dtype != icode::UI8)
            return false;

        return str.dimensions[0] <= assignee.dimensions[0];
    }

    bool isSameDim(TypeDescription type1, TypeDescription type2)
    {
        if (type1.isString() && !type2.isString())
            return canAssignString(type2, type1);

        if (!type1.isString() && type2.isString())
            return canAssignString(type1, type2);

        return type1.dimensions.size() == type2.dimensions.size();
    }

    bool isSameType(TypeDescription type1, TypeDescription type2)
    {
        if (type1.dtype == STRUCT || type2.dtype == STRUCT)
            return (type1.dtypeName == type2.dtypeName && isSameDim(type1, type2) &&
                    type1.moduleName == type2.moduleName);

        return (dataTypeIsEqual(type1.dtype, type2.dtype) && isSameDim(type1, type2));
    }
}