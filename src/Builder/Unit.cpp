#include "Unit.hpp"

Unit::Unit() {}

Unit::Unit(const icode::Operand& operand, const icode::TypeDescription& type)
{
    this->op = operand;
    this->type = type;
}