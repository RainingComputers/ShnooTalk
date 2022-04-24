#include "Unit.hpp"

using namespace icode;

Unit::Unit() {}

Unit::Unit(const TypeDescription& type, const Operand& operand)
{
    this->operand = operand;
    this->typeDescription = type;
}

Unit::Unit(const TypeDescription& type, const std::vector<Unit>& list)
{
    this->typeDescription = type;
    this->list = list;
};

std::vector<Unit> Unit::flatten() const
{
    std::vector<Unit> flatList;

    for (const Unit& childUnit : list)
    {
        if (childUnit.list.size() == 0)
            flatList.push_back(childUnit);
        else
        {
            std::vector<Unit> childFlatList = childUnit.flatten();
            flatList.insert(flatList.end(), childFlatList.begin(), childFlatList.end());
        }
    }

    return flatList;
}

Unit Unit::clearProperties()
{
    typeDescription.properties &= 1 << IS_PTR;
    return *this;
}

std::string Unit::moduleName() const
{
    return typeDescription.moduleName;
}

DataType Unit::dtype() const
{
    return typeDescription.dtype;
}

std::string Unit::dtypeName() const
{
    return typeDescription.dtypeName;
}
std::vector<int> Unit::dimensions() const
{
    return typeDescription.dimensions;
}

TypeDescription Unit::type() const
{
    return typeDescription;
}

Operand Unit::op() const
{
    return operand;
}

std::vector<Unit> Unit::destructureUnitList() const
{
    return list;
}

unsigned int Unit::size() const
{
    return typeDescription.size;
}

unsigned int Unit::dtypeSize() const
{
    return typeDescription.dtypeSize;
}

unsigned int Unit::numElements() const
{
    return typeDescription.dimensions[0];
}

bool Unit::isStringLtrl() const
{
    return typeDescription.checkProperty(IS_STRING_LTRL);
}

bool Unit::isChar() const
{
    return typeDescription.dtype == UI8;
}

bool Unit::isMutable() const
{
    return typeDescription.isMutable();
}

bool Unit::isIntegerType() const
{
    return typeDescription.isIntegerType();
}

bool Unit::isFloatType() const
{
    return typeDescription.isFloatType();
}

bool Unit::isStruct() const
{
    return typeDescription.isStruct();
}

bool Unit::isArray() const
{
    return typeDescription.isArray();
}

bool Unit::isStructOrArray() const
{
    return typeDescription.isStructOrArray();
}

bool Unit::isMultiDimArray() const
{
    return typeDescription.isMultiDimArray();
}

bool Unit::isLocal() const
{
    return typeDescription.checkProperty(IS_LOCAL);
}

bool Unit::isGlobal() const
{
    return typeDescription.checkProperty(IS_GLOBAL);
}

bool Unit::isLocalOrGlobal() const
{
    return isLocal() || isGlobal();
}

bool Unit::isValue() const
{
    return !isUserPointer() && isLocalOrGlobal();
}

bool Unit::isPointer() const
{
    return operand.isPointer();
}

bool Unit::isValidForPointerAssignment() const
{
    return operand.isValidForPointerAssignment();
}

bool Unit::isUserPointer() const
{
    return operand.isUserPointer();
}

bool Unit::isMutableAndPointer() const
{
    return typeDescription.isMutableAndPointer();
}

bool Unit::isMutableOrPointer() const
{
    return typeDescription.isMutableOrPointer();
}

bool Unit::isLiteral() const
{
    return operand.operandType == LITERAL;
}

bool Unit::isList() const
{
    return list.size() > 0;
}
