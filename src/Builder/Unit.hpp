#ifndef BUILDER_UNIT
#define BUILDER_UNIT

#include <utility>

#include "../IntermediateRepresentation/Operand.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"

struct Unit
{
    icode::Operand op;
    icode::TypeDescription type;

    Unit();
    Unit(const icode::Operand& operand, const icode::TypeDescription& type);
};

#endif