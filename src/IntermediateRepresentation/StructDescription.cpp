#include "GetMapElement.hpp"

#include "StructDescription.hpp"

namespace icode
{
    bool StructDescription::fieldExists(const std::string& name) const
    {
        return structFields.find(name) != structFields.end();
    }

    bool StructDescription::getField(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(structFields, name, returnValue);
    }

    DataType StructDescription::getFirstFieldDataType() const
    {
        if (structFields.size() == 0)
            return STRUCT;

        return structFields.begin()->second.dtype;
    }
}
