#ifndef IR_TYPE_DESCRIPTION
#define IR_TYPE_DESCRIPTION

#include <vector>

#include "DataType.hpp"
#include "TargetDescription.hpp"

namespace icode
{
    enum TypeProperties
    {
        IS_MUT,
        IS_PTR,
        IS_PARAM,
        IS_GLOBAL,
        IS_LOCAL,
        IS_ENUM,
        IS_DEFINE,
        IS_STRING_LTRL,
    };

    struct TypeDescription
    {
        DataType dtype;
        std::string dtypeName;
        std::string moduleName;
        unsigned int dtypeSize;
        unsigned int offset;
        unsigned int size;
        std::vector<unsigned int> dimensions;

        unsigned int properties;

        TypeDescription();

        void setProperty(TypeProperties prop);
        void clearProperty(TypeProperties prop);
        bool checkProperty(TypeProperties prop) const;

        void becomeMutable();
        void becomePointer();
        void becomeString();
        bool isMutable() const;
        bool isPointer() const;
        bool isString() const;

        bool isSameType() const;
        bool isStruct() const;
        bool isArray() const;
        bool isMultiDimArray() const;
        bool isIntegerType() const;
    };

    bool isSameType(TypeDescription var1, TypeDescription var2);
}

#endif