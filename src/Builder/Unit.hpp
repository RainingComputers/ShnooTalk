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

    std::string moduleName() const;

    icode::DataType dtype() const;
    std::string dtypeName() const;
    std::vector<int> dimensions() const;
    icode::TypeDescription type() const;
    icode::Operand op() const;

    int size() const;
    int dtypeSize() const;

    bool isStringLtrl() const;
    bool isChar() const;

    bool isMutable() const;

    bool isIntegerType() const;

    bool isStruct() const;
    bool isArray() const;
    bool isMultiDimArray() const;

    bool isEnum() const;
    bool isDefine() const;
    bool isLocal() const;

    bool isPointer() const;

    bool isInvalidForInput() const;
    bool canPassAsMutable() const;
    bool isLiteral() const;

    bool isList() const;
};

#endif