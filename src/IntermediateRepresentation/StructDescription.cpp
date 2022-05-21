#include "../Utils/GetMapElement.hpp"
#include "../Utils/KeyExistsInMap.hpp"

#include "StructDescription.hpp"

namespace icode
{
    bool StructDescription::fieldExists(const std::string& name) const
    {
        return keyExistsInMap(structFields, name);
    }

    DataType StructDescription::getFirstFieldDataType() const
    {
        if (structFields.size() == 0)
            return STRUCT;

        return structFields.begin()->second.dtype;
    }
}
