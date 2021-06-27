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
    std::vector<Unit> flattenedUnitList;

    for (const Unit& childUnit : aggUnit.aggs)
    {
        if (childUnit.aggs.size() == 0)
            flattenedUnitList.push_back(childUnit);
        else
        {
            std::vector<Unit> childFlattendUnitList = flattenUnit(childUnit);
            flattenedUnitList.insert(flattenedUnitList.end(),
                                     childFlattendUnitList.begin(),
                                     childFlattendUnitList.end());
        }
    }

    return flattenedUnitList;
}

std::vector<Unit> Unit::flatten() const
{
    return flattenUnit(*this);
}