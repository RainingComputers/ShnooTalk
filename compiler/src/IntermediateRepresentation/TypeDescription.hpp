#ifndef IR_TYPE_DESCRIPTION
#define IR_TYPE_DESCRIPTION

#include <vector>

#include "DataType.hpp"

namespace icode
{
    enum TypeProperties
    {
        IS_MUT,
        IS_PTR,
        IS_PARAM,
        IS_GLOBAL,
        IS_LOCAL,
        IS_STRING_LTRL,
        IS_INCOMPLETE_TYPE
    };

    enum DimensionType
    {
        FIXED_DIM,
        STRING_LTRL_DIM,
        ALLOC_FLEXIBLE_DIM,
    };

    struct TypeDescription
    {
        DataType dtype;
        std::string dtypeName;
        std::string moduleName;
        unsigned int dtypeSize;
        unsigned int offset;
        unsigned int size;
        std::vector<int> dimensions;
        std::vector<DimensionType> dimTypes;

        unsigned int properties;

        TypeDescription();

        void setProperty(TypeProperties prop);
        void clearProperty(TypeProperties prop);
        bool checkProperty(TypeProperties prop) const;

        void becomeMutable();
        void becomePointer();
        void becomeArrayPointer();
        void becomeStringLtrl();
        void becomeImmutable();
        void becomeNonPointer();
        void becomeIncompleteType();

        void decayAutoType();

        bool isMutable() const;
        bool isPassedByReference() const;
        bool isPointer() const;
        bool isMutableAndPointer() const;
        bool isMutableOrPointer() const;
        bool isStringLtrl() const;
        bool isIncompleteType() const;

        bool isPrimitiveType() const;
        bool isEnum() const;
        bool isStruct() const;
        bool isArray() const;
        bool isArrayWithFixedDim() const;
        bool isStructOrArray() const;
        bool isStructOrArrayAndNotPointer() const;
        bool isMultiDimArray() const;
        bool isIntegerType() const;
        bool isFloatType() const;
        bool isVoid() const;

        unsigned int numElements() const;
    };
}

#endif