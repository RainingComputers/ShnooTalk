#include "StructDescription.hpp"

namespace icode
{
    bool StructDescription::fieldExists(const std::string& name)
    {
        return structFields.find(name) != structFields.end();
    }
}
