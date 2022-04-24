#ifndef BUILDER_UNIT
#define BUILDER_UNIT

#include <utility>

#include "../IntermediateRepresentation/Operand.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"

class Unit
{
    icode::TypeDescription typeDescription;
    icode::Operand operand;
    std::vector<Unit> list;

public:
    Unit();
    Unit(const icode::TypeDescription& type, const icode::Operand& operand);
    Unit(const icode::TypeDescription& type, const std::vector<Unit>& list);
    std::vector<Unit> flatten() const;
    Unit clearProperties();

    std::string moduleName() const;

    icode::DataType dtype() const;
    std::string dtypeName() const;
    std::vector<int> dimensions() const;
    icode::TypeDescription type() const;
    icode::Operand op() const;
    std::vector<Unit> destructureUnitList() const;

    unsigned int size() const;
    unsigned int dtypeSize() const;
    unsigned int numElements() const;

    bool isStringLtrl() const;
    bool isChar() const;

    bool isMutable() const;

    bool isIntegerType() const;
    bool isFloatType() const;

    bool isStruct() const;
    bool isArray() const;
    bool isStructOrArray() const;
    bool isMultiDimArray() const;
    bool isArrayWithFixedDim() const;

    bool isEnum() const;
    bool isLocal() const;
    bool isGlobal() const;
    bool isLocalOrGlobal() const;
    bool isValue() const;

    bool isPointer() const;

    bool isValidForPointerAssignment() const;
    bool isUserPointer() const;

    bool isMutableAndPointer() const;
    bool isMutableOrPointer() const;

    bool isLiteral() const;

    bool isList() const;
};

#endif