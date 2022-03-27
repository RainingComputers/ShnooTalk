#include "../Utils/GetMapElement.hpp"

#include "StructDescription.hpp"

namespace icode
{
    bool StructDescription::fieldExists(const std::string& name) const
    {
        return structFields.find(name) != structFields.end();
    }

    DataType StructDescription::getFirstFieldDataType() const
    {
        if (structFields.size() == 0)
            return STRUCT;

        return structFields.begin()->second.dtype;
    }
}
