#include "Unit.hpp"

using namespace icode;

Unit::Unit() {}

Unit::Unit(const TypeDescription& type, const Operand& operand)
{
    this->op = operand;
    this->type = type;
}

Unit::Unit(const TypeDescription& type, const std::vector<Unit>& aggs)
{
    this->type = type;
    this->aggs = aggs;
};

std::vector<Unit> flattenUnit(const Unit& aggUnit)
{
    std::vector<Unit> flatList;

    for (const Unit& childUnit : aggUnit.aggs)
    {
        if (childUnit.aggs.size() == 0)
            flatList.push_back(childUnit);
        else
        {
            std::vector<Unit> childFlatList = flattenUnit(childUnit);
            flatList.insert(flatList.end(), childFlatList.begin(), childFlatList.end());
        }
    }

    return flatList;
}

std::vector<Unit> Unit::flatten() const
{
    return flattenUnit(*this);
}