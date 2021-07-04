#include "Unit.hpp"

using namespace icode;

Unit::Unit() {}

Unit::Unit(const TypeDescription& type, const Operand& operand)
{
    this->op = operand;
    this->type = type;
}

Unit::Unit(const TypeDescription& type, const std::vector<Unit>& list)
{
    this->type = type;
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