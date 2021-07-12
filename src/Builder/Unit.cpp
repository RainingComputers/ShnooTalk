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

int Unit::size() const
{
    return typeDescription.size;
}

int Unit::dtypeSize() const
{
    return typeDescription.dtypeSize;
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

bool Unit::isStruct() const
{
    return typeDescription.isStruct();
}

bool Unit::isArray() const
{
    return typeDescription.isArray();
}

bool Unit::isMultiDimArray() const
{
    return typeDescription.isMultiDimArray();
}

bool Unit::isEnum() const
{
    return typeDescription.checkProperty(IS_ENUM);
}

bool Unit::isDefine() const
{
    return typeDescription.checkProperty(IS_DEFINE);
}

bool Unit::isLocal() const
{
    return typeDescription.checkProperty(IS_LOCAL);
}

bool Unit::isPointer() const
{
    return operand.isPointer();
}

bool Unit::isInvalidForInput() const
{
    return operand.isInvalidForInput();
}

bool Unit::canPassAsMutable() const
{
    return operand.canPassAsMutable();
}

bool Unit::isLiteral() const
{
    return operand.operandType == LITERAL;
}

bool Unit::isList() const
{
    return list.size() > 0;
}