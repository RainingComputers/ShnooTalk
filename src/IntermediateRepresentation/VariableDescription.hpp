#ifndef VARIABLE_DESCRIPTION
#define VARIABLE_DESCRIPTION

#include <vector>

#include "DataType.hpp"
#include "TargetDescription.hpp"

namespace icode
{
    enum VariableProperty
    {
        IS_MUT,
        IS_PTR,
        IS_PARAM,
    };

    struct VariableDescription
    {
        DataType dtype;
        std::string dtypeName;
        std::string moduleName;
        unsigned int dtypeSize;
        unsigned int offset;
        unsigned int size;
        std::vector<unsigned int> dimensions;
        unsigned int scopeId;

        unsigned int properties;

        VariableDescription();

        void setProperty(VariableProperty prop);
        void clearProperty(VariableProperty prop);
        bool checkProperty(VariableProperty prop) const;
    };

    bool isSameType(VariableDescription var1, VariableDescription var2);

    // TODO Remove this
    VariableDescription variableDescriptionFromDataType(DataType dtype, TargetDescription& target);
}

#endif